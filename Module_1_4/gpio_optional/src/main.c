#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_timer.h>

#define RED_LED_PIN 15
#define GREEN_LED_PIN 16
#define EXTERNAL_BUTTON_PIN 17
#define BOOT_BUTTON_PIN 0

#define DEBOUNCE_DELAY 200 // 200ms
#define CONTROLLER_SCAN_PERIOD 100 //100ms
#define NUMBER_OF_SPEED_MODES 4

const int blink_speeds[NUMBER_OF_SPEED_MODES] = {1000, 500, 250, 125};

typedef enum {
    SERIAL = 0,
    SIMULTANEOUS = 1
} blink_mode;

volatile blink_mode current_blink_mode = SERIAL;
volatile int current_blink_speed_idx = 0;

volatile int last_boot_triggered_time = 0;
volatile int last_external_triggered_tIme = 0;

TaskHandle_t blink_task = NULL; 
TaskHandle_t input_controller_task = NULL;

void IRAM_ATTR boot_button_isr(void *arg) {
    int now = esp_timer_get_time();
    if (now - last_boot_triggered_time > DEBOUNCE_DELAY*1000) {
        last_boot_triggered_time = now;
    }
}

void IRAM_ATTR external_button_isr(void *arg) {
    int now = esp_timer_get_time();
    if (now - last_external_triggered_tIme > DEBOUNCE_DELAY*1000) {
        last_external_triggered_tIme = now;
    }
}

// runs each 100ms
void input_controller(void *pvParameters) {
    static int last_controller_run = 0;
    int now = esp_timer_get_time();
    int state = 0; 

    TickType_t last_wake_time;
    const TickType_t task_period_ticks = pdMS_TO_TICKS(CONTROLLER_SCAN_PERIOD);
    while (1) {
        vTaskDelayUntil(&last_wake_time, task_period_ticks);
        state = 0;    
        // controller check sor button press registred in time period beetween
        // last_controller_run and now

        // button was pressed in sanned period set first bit
        if (last_boot_triggered_time > last_controller_run && last_boot_triggered_time < now) {
            state += (1 << 0);
        }

        // button was pressed in sanned period set second bit
        if (last_external_triggered_tIme > last_controller_run && last_external_triggered_tIme < now) {
            state += (1 << 0);
        }

        last_controller_run = now;

        switch (state) {    
            case 0: // if none were pressed - skip
                break;
            case 1: // if boot was predded - next speed
                current_blink_speed_idx = (current_blink_speed_idx + 1) % NUMBER_OF_SPEED_MODES;
                break;
            case 2: // if external was predded - previous speed
                current_blink_speed_idx = current_blink_speed_idx - 1 >= 0 ? current_blink_speed_idx - 1 : NUMBER_OF_SPEED_MODES-1 ;
                break;
            case 3: // if both buttons were pressed in scanned period - perform mode change
                // we have only two modes so reverting value is enough
                // 0001 || 1110 = 1111 => !1111 = 0000
                // 0000 || 1110 = 1110 => !1110 = 0001
                current_blink_mode = !(current_blink_mode || !1);
                break;
        }
    }
}

void serial_blink(int delay) {
    gpio_set_level(GREEN_LED_PIN, 0);
    gpio_set_level(RED_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(delay));

    gpio_set_level(GREEN_LED_PIN, 1);
    gpio_set_level(RED_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(delay));

    gpio_set_level(GREEN_LED_PIN, 0);
}

void simultaneous_blink(int delay) {
    gpio_set_level(GREEN_LED_PIN, 1);
    gpio_set_level(RED_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(delay));

    gpio_set_level(GREEN_LED_PIN, 0);
    gpio_set_level(RED_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(delay));
}

void blink(void *pvParameters) {
    while (1)
    {
        int delay = blink_speeds[current_blink_speed_idx];
        switch (current_blink_mode) {
            case SERIAL: 
                serial_blink(delay);
                break;
            case SIMULTANEOUS:
                simultaneous_blink(delay);
                break;
        }
    }
}

void gpio_init() {
    gpio_config_t red_led_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1 << RED_LED_PIN
    };
    gpio_config(&red_led_conf);

    gpio_config_t green_led_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1 << GREEN_LED_PIN
    };
    gpio_config(&green_led_conf);

    gpio_config_t external_button_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1 << EXTERNAL_BUTTON_PIN
    };
    gpio_config(&external_button_conf);

    gpio_config_t boot_button_conf = {  
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1 << BOOT_BUTTON_PIN
    };
    gpio_config(&boot_button_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(0);

    // Add ISR handler for buttons
    gpio_isr_handler_add(BOOT_BUTTON_PIN, boot_button_isr, NULL);
    gpio_isr_handler_add(EXTERNAL_BUTTON_PIN, external_button_isr, NULL);

    xTaskCreatePinnedToCore(
        blink,
        "Blink",
        2048,
        NULL,
        1,
        &blink_task,
        1
    );

    xTaskCreatePinnedToCore(
        input_controller,
        "Input Controller",
        2048,
        NULL,
        4,
        &input_controller_task,
        1
    );
}

void app_main() {
    gpio_init();

    while (1)
    {
        int blink_delay = blink_speeds[current_blink_speed_idx];
        switch (current_blink_mode) {
            case SERIAL:
                serial_blink(blink_delay);
                break;
            case SIMULTANEOUS:
                simultaneous_blink(blink_delay);
                break;
        }
    }
}