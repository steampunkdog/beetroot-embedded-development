#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include <esp_timer.h>
#include "esp_log.h"

#define OUTPUT_PIN 15
#define INPUT_PIN 16
#define MAX_STORED_TIMESTAMPS 20
#define COLLECT_RESULTS_DELAY 1000
#define MEASUREMENTS_DELAY 1000
#define MEASUREMENTS_TAG "measurement"


static volatile int64_t isr_triggered_timestamps[MAX_STORED_TIMESTAMPS] = {};
static volatile size_t writer_index = 0;
static volatile bool overflow = false;

void handle_relay_input(void *arg) {
    if (writer_index == MAX_STORED_TIMESTAMPS) {
        overflow = true;
        return;
    }
    isr_triggered_timestamps[writer_index] = esp_timer_get_time();
    writer_index++;
}

void clear_timestamps() {
    for (size_t i = 0; i < MAX_STORED_TIMESTAMPS; i++) {
        isr_triggered_timestamps[i] = 0;
    }
    writer_index = 0;
    overflow = false;
}

void init() {
    gpio_config_t out_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = 1 << OUTPUT_PIN,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE 
    };
    gpio_config(&out_conf);

    gpio_config_t in_conf = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE,
        .pin_bit_mask = 1 << INPUT_PIN,
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // pulldown is added to circuit
        .pull_up_en = GPIO_PULLUP_DISABLE 
    };
    gpio_config(&in_conf);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(INPUT_PIN, handle_relay_input, NULL);

}

void app_main() {
    init();
    int iteration = 1;
    while (1) {
        static int set_pin_to = 1;

        ESP_LOGI(MEASUREMENTS_TAG, "-----------------------------------------");
        ESP_LOGI(MEASUREMENTS_TAG, "Iteration %d", iteration);

        gpio_set_level(OUTPUT_PIN, set_pin_to);
        uint64_t pin_set_timestamp = esp_timer_get_time();
        ESP_LOGI(MEASUREMENTS_TAG, "Pin level set to %d, at %lld. Collecting input...", set_pin_to, pin_set_timestamp);

        vTaskDelay(pdMS_TO_TICKS(COLLECT_RESULTS_DELAY));

        for (size_t i = 0; i < writer_index; i++) {
            if(i == 0) {
                ESP_LOGI(MEASUREMENTS_TAG, "Time to first switch: %lld", isr_triggered_timestamps[i] - pin_set_timestamp);
            }

            ESP_LOGI(MEASUREMENTS_TAG, "State changed i: %zu at: %lld", i, isr_triggered_timestamps[i]);

            if(i + 1 == writer_index) {
                ESP_LOGI(MEASUREMENTS_TAG, "Time to stable state: %lld", isr_triggered_timestamps[i] - pin_set_timestamp);
            }
        }

        if (overflow) {
            ESP_LOGW(MEASUREMENTS_TAG, "Got overflow");
        }
        
        clear_timestamps();
        set_pin_to = !set_pin_to;

        vTaskDelay(pdMS_TO_TICKS(MEASUREMENTS_DELAY));
        iteration++;
    }
}