#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_timer.h>
#include "esp_log.h"

#define RED_LED_PIN 15
#define GREEN_LED_PIN 16

#define NEXT_BUTTON_PIN 0   // BOOT button
#define PREV_BUTTON_PIN 17  // EXTERNAL button

#define DEBOUNCE_DELAY_MS 200
#define CONTROLLER_SCAN_PERIOD_MS 200

// Tags for logging
#define MAIN_APP_TAG "main"
#define CONTROLLER_TAG "controller"

// Suppored blinking modes
typedef enum {
    SERIAL = 0,       // LEDs flash in sequence
    SIMULTANEOUS = 1  // LEDs flash at the same time
} blink_mode;
volatile blink_mode current_blink_mode = SERIAL;


// 4 blinkng speeds are supported
#define NUMBER_OF_SPEED_MODES 4 
const int blink_speeds[NUMBER_OF_SPEED_MODES] = {1000, 500, 250, 125}; // Stores speeds in ms
volatile int current_blink_speed_idx = 0;

// last_*_triggered_time vars store the last timestamp when the corresponding buttons "click" event was registered 
volatile int last_next_triggered_time = 0;
volatile int last_prev_triggered_time = 0;

// Interruption handler for NEXT button 
void IRAM_ATTR next_button_isr(void *arg) {
    int now = esp_timer_get_time();
    if (now - last_next_triggered_time > DEBOUNCE_DELAY_MS*1000) {
        last_next_triggered_time = now;
    }
}

// Interruption handler for PREV button
void IRAM_ATTR prev_button_isr(void *arg) {
    int now = esp_timer_get_time();
    if (now - last_prev_triggered_time > DEBOUNCE_DELAY_MS*1000) {
        last_prev_triggered_time = now;
    }
}

/*  
    This task reads last_*_triggered_time variables once in CONTROLLER_SCAN_PERIOD_MS
    if on one both buttons were pressed beetween previous controller iteration and current time
    controller applys the effect associated with this button's(or combination) by changing the values of  
    current_blink_mode and current_blink_speed_idx variables
*/
void input_controller(void *pvParameters) {
    static int last_controller_run = 0;
    int now = 0;
    int state = 0; //first bit - NEXT button, seconf bit - PREV button

    ESP_LOGI(CONTROLLER_TAG, "Controller initialized");

    // controller check for button press registred in time period beetween last_controller_run and now
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(CONTROLLER_SCAN_PERIOD_MS));
        state = 0;
        now = esp_timer_get_time();

        // NEXT button was pressed in scanned period - set first bit
        if (last_next_triggered_time > last_controller_run && last_next_triggered_time < now) {
            state += (1 << 0);
        }

        // PREV button was pressed in scanned period - set second bit
        if (last_prev_triggered_time > last_controller_run && last_prev_triggered_time < now) {
            state += (1 << 1);
        }

        last_controller_run = now;

        
        switch (state) {    
            case 0: // if none were pressed - skip
                break;
            case 1: // if NEXT was pressed - next speed
                current_blink_speed_idx = (current_blink_speed_idx + 1) % NUMBER_OF_SPEED_MODES;
                ESP_LOGI(CONTROLLER_TAG, "Got 'NEXT' signal. Switching to speed %d", current_blink_speed_idx);
                break;
            case 2: // if PREV was pressed - previous speed
                current_blink_speed_idx = current_blink_speed_idx - 1 >= 0 ? current_blink_speed_idx - 1 : NUMBER_OF_SPEED_MODES-1;
                ESP_LOGI(CONTROLLER_TAG, "Got 'PREV' signal. Switching to speed %d", current_blink_speed_idx);
                break;
            case 3: // if both buttons were pressed in scanned period - perform mode change
                current_blink_mode = !(current_blink_mode || !1); // we have only two modes(0 and 1) so reverting value is enough
                ESP_LOGI(CONTROLLER_TAG, "Got 'MODE' signal. Switching to mode %d", current_blink_mode);
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

// Task handles
TaskHandle_t blink_task = NULL; 
TaskHandle_t input_controller_task = NULL;

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
        .pin_bit_mask = 1 << PREV_BUTTON_PIN,
        .pull_up_en = GPIO_PULLUP_DISABLE //External button has build-in pull-up resistor
    };
    gpio_config(&external_button_conf);

    gpio_config_t boot_button_conf = {  
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1 << NEXT_BUTTON_PIN,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&boot_button_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(0);

    // Add ISR handler for buttons
    gpio_isr_handler_add(NEXT_BUTTON_PIN, next_button_isr, NULL);
    gpio_isr_handler_add(PREV_BUTTON_PIN, prev_button_isr, NULL);

    // Create and run blink task
    xTaskCreatePinnedToCore(
        blink,
        "Blink",
        2048,
        NULL,
        1,
        &blink_task,
        1
    );

    // Create and run controller task
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
    ESP_LOGI(MAIN_APP_TAG, "initialized");
}