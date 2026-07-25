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

#define DEBOUNCE_DELAY 200000 // 200ms

typedef enum {
    NONE = 0,
    SLOW = 1,
    FAST = 2
} blink_mode;

volatile blink_mode current_blink_mode = NONE;

void IRAM_ATTR boot_button_isr(void *arg) {
    static int lastTriggeredTime = 0;
    int now = esp_timer_get_time();
    if (now - lastTriggeredTime > DEBOUNCE_DELAY) {
        lastTriggeredTime = now;
        current_blink_mode = SLOW;
    }
}

void IRAM_ATTR external_button_isr(void *arg) {
    static int lastTriggeredTime = 0;
    int now = esp_timer_get_time();
    if (now - lastTriggeredTime > DEBOUNCE_DELAY) {
        lastTriggeredTime = now;
        current_blink_mode = FAST;
    }
}

void blink(int delay) {
    gpio_set_level(GREEN_LED_PIN, 0);
    gpio_set_level(RED_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(delay));

    gpio_set_level(GREEN_LED_PIN, 1);
    gpio_set_level(RED_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(delay));

    gpio_set_level(GREEN_LED_PIN, 1);
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
}

void app_main() {
    gpio_init();

    while (1)
    {
        switch (current_blink_mode) {
            case SLOW:
                blink(1000);
                break;
            case FAST:
                blink(300);
                break;
            case NONE:
                break;
        }
    }
}