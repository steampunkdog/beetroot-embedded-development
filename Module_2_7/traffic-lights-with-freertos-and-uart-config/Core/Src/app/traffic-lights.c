#include "traffic-lights.h"
#include "commands.h"
#include "logger.h"

#include "queue.h"
#include "task.h"

typedef struct {
	uint16_t pin_bit_mask;
	uint16_t duration_ticks;
	uint16_t period_ticks;
} phase_t;

static uint16_t traffic_pins_bitmask;

static phase_t regular_mode_phases[5];
static phase_t flasing_yellow_phase = {};

static QueueHandle_t command_queue_h;
static TaskHandle_t apply_command_task_h;
static TaskHandle_t traffic_lights_task_h;
static pins_config_t pin_config;

void apply_command_task(void *params);
void traffic_lights_task(void *params);
static void set_active_pins(uint16_t pin_bit_mask);
static void toggle_pins_state(uint16_t pin_bit_mask);
static void fill_phases_from_config(traffic_config_t *config);

void traffic_lights_init(QueueHandle_t* command_queue_p, pins_config_t *config) {
	pin_config = *config;
	traffic_pins_bitmask =
			1 << pin_config.green_light_pin
			| 1 << pin_config.yellow_light_pin
			| 1 << pin_config.red_light_pin;

	*command_queue_p = xQueueGenericCreate(5, sizeof(command_t*), queueQUEUE_TYPE_BASE);
	command_queue_h = *command_queue_p;

	xTaskCreate(
		apply_command_task,
		"apply_command_task",
		256,
		NULL,
		15,
		&apply_command_task_h
	);

	xTaskCreate(
		traffic_lights_task,
		"traffic_lights_task",
		512,
		NULL,
		10,
		&traffic_lights_task_h
	);
}

void apply_command_task(void *params) {
	command_t *command;
	while(1) {
		xQueueReceive(command_queue_h, &command, portMAX_DELAY);
		log_to_serial("Accepting command %d\r\n", command->command);
		xTaskNotify(traffic_lights_task_h, command->command, eSetValueWithOverwrite);
		if (command->command == COMMAND_CONFIG) {
			fill_phases_from_config(command->config);
		}

		free_command(command);
	}
}

typedef struct {
	phase_t* phases;
	size_t number_of_phases;

	TickType_t phase_started;
	size_t phase_idx;
	phase_t phase;
	int number_of_blinks;

	command_types last_command;
} traffic_lights_task_state_t;

static traffic_lights_task_state_t cts;

static void reset_traffic_lights_task_state(phase_t* phases, size_t number_of_phases) {
	cts.phases = phases;
	cts.number_of_phases = number_of_phases;

	cts.phase_idx = 0;
	cts.phase_started = xTaskGetTickCount();
	cts.phase = cts.phases[cts.phase_idx];
	cts.number_of_blinks = 0;

	// to avoid writing init logic in cycle
	set_active_pins(cts.phase.pin_bit_mask);
}

void traffic_lights_task(void *params) {
	cts.last_command = COMMAND_NONE;

	while (1) {
		// handle incoming command
		uint32_t notification = ulTaskNotifyTake(pdTRUE, 0);
		if (notification) {
			if (cts.last_command == notification) {
				log_to_serial("Already in requested state \r\n");
			} else if (notification == COMMAND_RUN) {
				reset_traffic_lights_task_state(regular_mode_phases, sizeof(regular_mode_phases)/sizeof(phase_t));
				log_to_serial("Running \r\n");
			} else if (notification == COMMAND_FLASHING_YELLOW) {
				reset_traffic_lights_task_state(&flasing_yellow_phase, 1);
				log_to_serial("Flashing Yellow \r\n");
			} else if (notification == COMMAND_STOP) {
				// Do nothing, while waiting for next state change
				set_active_pins(0);
				log_to_serial("Stopped \r\n");
			} else if (notification == COMMAND_CONFIG) {
				// Do nothing, while waiting for next state change
				set_active_pins(0);
				log_to_serial("Configuration \r\n");
			}
			cts.last_command = notification;
		}

		// Regular logic
		if (cts.last_command == COMMAND_RUN) {
			TickType_t now = xTaskGetTickCount();
			if (now - cts.phase_started >= cts.phase.duration_ticks) {
				cts.phase_idx = (cts.phase_idx + 1) % cts.number_of_phases;
				cts.phase = cts.phases[cts.phase_idx];
				cts.phase_started = xTaskGetTickCount();\
				cts.number_of_blinks = 0;

				set_active_pins(cts.phase.pin_bit_mask);
			} else if (cts.phase.period_ticks != 0 && (now - cts.phase_started) / cts.phase.period_ticks > cts.number_of_blinks) {
				toggle_pins_state(cts.phase.pin_bit_mask);
				cts.number_of_blinks++;
			}
		}

		// Flashing Yellow logic
		if (cts.last_command == COMMAND_FLASHING_YELLOW) {
			TickType_t now = xTaskGetTickCount();
			if ((now - cts.phase_started) / cts.phase.period_ticks > cts.number_of_blinks)  {
				toggle_pins_state(cts.phase.pin_bit_mask);
				cts.number_of_blinks++;
			}
 		}

		vTaskDelay(10);
	}
}

static void set_active_pins(uint16_t pin_bit_mask) {
	HAL_GPIO_WritePin(pin_config.gpio_port, pin_bit_mask, 1);
	uint16_t pins_to_disable = traffic_pins_bitmask & ~pin_bit_mask;
	HAL_GPIO_WritePin(pin_config.gpio_port, pins_to_disable, 0);
}

static void toggle_pins_state(uint16_t pin_bit_mask) {
	HAL_GPIO_TogglePin(pin_config.gpio_port, pin_bit_mask);
}

static void fill_phases_from_config(traffic_config_t *config) {
	// Green
	regular_mode_phases[0] = (phase_t){
		.pin_bit_mask = 1 << pin_config.green_light_pin,
		.duration_ticks = config->green_ms,
		.period_ticks = 0
	};

	// Green flashing
	regular_mode_phases[1] = (phase_t){
		.pin_bit_mask = 1 << pin_config.green_light_pin,
		.duration_ticks = config->green_flashing_ms,
		.period_ticks = pdMS_TO_TICKS(config->green_flashing_period)
	};

	// Yellow
	regular_mode_phases[2] = (phase_t){
		.pin_bit_mask = 1 << pin_config.yellow_light_pin,
		.duration_ticks = config->yellow_ms,
		.period_ticks = 0
	};

	// Red
	regular_mode_phases[3] = (phase_t){
		.pin_bit_mask = 1 << pin_config.red_light_pin,
		.duration_ticks = config->red_ms,
		.period_ticks = 0
	};

	// Red and Yellow
	regular_mode_phases[4] = (phase_t){
		.pin_bit_mask = 1 << pin_config.yellow_light_pin | 1 << pin_config.red_light_pin,
		.duration_ticks = config->red_and_yellow_ms,
		.period_ticks = 0
	};

	// Yellow flashing
	flasing_yellow_phase.pin_bit_mask = 1 << pin_config.yellow_light_pin;
	flasing_yellow_phase.duration_ticks = UINT16_MAX;
	flasing_yellow_phase.period_ticks = config->flashing_yellow_period;
}
