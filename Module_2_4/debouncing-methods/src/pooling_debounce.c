#include "driver/gpio.h"
#include <esp_timer.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

typedef enum {
    LOW,
    HIGH
} pin_state;

static void init(const uint32_t button_pin) {
    gpio_config_t button_conf = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1 << button_pin)
    };
    gpio_config(&button_conf);
}

void pooling_debounce_app(const int button_pin) {
    init(button_pin);

    int counter = 0;
    pin_state current_state = HIGH;
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(10));

        uint32_t new_state = gpio_get_level(button_pin);

        //react only if state changes
        if (new_state != current_state) {
            // counter is incresed only when signal goes from HIGH state to LOW
            // other cases are ignored
            if (current_state == HIGH && new_state == LOW) {
                ESP_LOGI("no-debounce", "Counter: %d", ++counter);
            }
            current_state = new_state;
        }
    }
}