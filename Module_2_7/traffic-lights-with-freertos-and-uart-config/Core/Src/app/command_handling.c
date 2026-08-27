#include "command_handling.h"
#include "commands.h"
#include "logger.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "string.h"

#define COMMAND_BUFF_SIZE 8
static uint8_t dma_buff[COMMAND_BUFF_SIZE * 2] = {0};
static uint16_t command_buff[COMMAND_BUFF_SIZE] = {0};

static TaskHandle_t process_command_task_h;
static QueueHandle_t command_queue_h;
static UART_HandleTypeDef *huart = NULL;

static void process_command_task(void *params);
static traffic_config_t* create_config_from_buffer(uint16_t *values);
static void send_command(command_t* command);

void init_command_handler(UART_HandleTypeDef *huart_n, QueueHandle_t command_queue) {
	static int initialized = 0;
	if (initialized) {
		return;
	}

	huart = huart_n;

	command_queue_h = command_queue;

	BaseType_t task_created = xTaskCreate(
		process_command_task,
		"process_command_task",
		512,
		NULL,
		20,
		&process_command_task_h
	);

	if (task_created != pdPASS) {
		log_to_serial("ERROR: failed to create process_command_task_h. Code: %d\r\n", task_created);
		return;
	}

	HAL_StatusTypeDef dma_init = HAL_UARTEx_ReceiveToIdle_DMA(huart, dma_buff, sizeof(dma_buff));
	if (dma_init != HAL_OK) {
		log_to_serial("ERROR: failed to init UART to DMA. Code: %d\r\n", dma_init);
		return;
	} else {
		__HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
	}

	log_to_serial("Ready to accept commands.\r\n");

	initialized = 1;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1) {
    	memcpy(command_buff, dma_buff, Size);
    	memset(dma_buff, 0, Size);

		HAL_StatusTypeDef dma_init = HAL_UARTEx_ReceiveToIdle_DMA(huart, dma_buff, sizeof(dma_buff));
		if (dma_init != HAL_OK) {
			log_to_serial("ERROR: failed to re-init UART to DMA. Code: %d\r\n", dma_init);
			return;
		} else {
			__HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
		}

    	BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
    	xTaskNotifyFromISR(process_command_task_h, 0, eNoAction, &pxHigherPriorityTaskWoken);
    	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
    }
}

// TODO think of a way to avoid command_buff race overrides
static void process_command_task(void *params) {
	while(1) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		uint16_t command_bit = command_buff[0];
		if (command_bit > 4) {
			log_to_serial("ERROR: invalid command start char\r\n");
			return;
		}

		command_t *command = NULL;
		switch (command_bit) {
			case COMMAND_RUN:
			case COMMAND_STOP:
			case COMMAND_FLASHING_YELLOW:
				command = alloc_and_create_command((command_types)command_bit, NULL);
				break;
			case COMMAND_CONFIG:
				command = alloc_and_create_command(
						(command_types)command_bit,
						create_config_from_buffer(&command_buff[1])
				);
				break;
		}

		log_to_serial("Got command %d\r\n", command->command);
		if (command->config != NULL) {
			traffic_config_t *config = command->config;
			log_to_serial("Got configuration: {");
			log_to_serial("\tgreen_ms = %d;\r\n", config->green_ms);
			log_to_serial("\tgreen_flashing_ms = %d;\r\n", config->green_flashing_ms);
			log_to_serial("\tgreen_flashing_period = %d;\r\n", config->green_flashing_period);
			log_to_serial("\tyellow_ms = %d;\r\n", config->yellow_ms);
			log_to_serial("\tred_ms = %d;\r\n", config->red_ms);
			log_to_serial("\tred_and_yellow_ms = %d;\r\n", config->red_and_yellow_ms);
			log_to_serial("\tflashing_yellow_period = %d}\r\n", config->flashing_yellow_period);
		}

		send_command(command);

		log_to_serial("Ready to accept commands.\r\n");
	}
}

static void send_command(command_t* command) {
	xQueueGenericSend(command_queue_h, &command, pdMS_TO_TICKS(10), queueSEND_TO_BACK);
}

static traffic_config_t* create_config_from_buffer(uint16_t *values) {
	traffic_config_t * config = alloc_and_create_config();

	config->green_ms = values[0];
	config->green_flashing_ms = values[1];
	config->green_flashing_period = values[2];
	config->yellow_ms = values[3];
	config->red_ms = values[4];
	config->red_and_yellow_ms = values[5];
	config->flashing_yellow_period = values[6];

	return config;
}
