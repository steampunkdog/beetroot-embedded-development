#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include <esp_timer.h>
#include "esp_log.h"

#define BUTTON_PIN 15
#define COUNTER_TAG "counter"

void button_isr_handler(void *args) {
    static uint8_t counter = 0;
    int64_t click_timer = esp_timer_get_time(); // save triggered time
    counter++; // increment counter
    ESP_EARLY_LOGI(COUNTER_TAG, "Button was pressed. Counter: %2d; Click time: %d", counter, click_timer); // log count and triggered time
}

void init() {
    gpio_config_t button = {
        .pin_bit_mask = 1 << BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE // pullup is built into button
    };
    gpio_config(&button);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

}

void app_main() {
    init();
}