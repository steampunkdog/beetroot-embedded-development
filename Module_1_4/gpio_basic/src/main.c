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
#define PREV_BUTTON_PIN 17 // EXTERNAL button 
#define NEXT_BUTTON_PIN 0  // BOOT button

#define DEBOUNCE_DELAY_MS 200

typedef enum {
    NONE = 0,
    SLOW = 1000,
    FAST = 300
} blink_mode;

volatile blink_mode current_blink_mode = NONE;

//Interruption function for BOOT button
void IRAM_ATTR next_button_isr(void *arg) {
    static int lastTriggeredTime = 0;
    int now = esp_timer_get_time();
    
    // If time between last signal and now is bigger than debounce delay - register new click
    if (now - lastTriggeredTime > DEBOUNCE_DELAY_MS * 1000) {
        lastTriggeredTime = now;
        current_blink_mode = SLOW;
    }
}

//Interruption function for EXTERNAL button
void IRAM_ATTR prev_button_isr(void *arg) {
    static int lastTriggeredTime = 0;
    int now = esp_timer_get_time();

    // If time between last signal and now is bigger than debounce delay - register new click  
    if (now - lastTriggeredTime > DEBOUNCE_DELAY_MS) {
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

    gpio_set_level(GREEN_LED_PIN, 0);
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
}

void app_main() {
    gpio_init();

    while (1) {
        if(current_blink_mode != NONE) {
            blink(current_blink_mode);
        }
    }
}