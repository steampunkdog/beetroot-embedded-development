#include "driver/gpio.h"
#include <esp_timer.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static volatile int counter = 0;

static void button_press_handler(void *args) {
    counter++;
}

static void init(const uint32_t button_pin) {
    gpio_config_t button_conf = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = (1 << button_pin)
    };

    ESP_ERROR_CHECK(gpio_config(&button_conf));

    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    ESP_ERROR_CHECK(gpio_isr_handler_add(button_pin, button_press_handler, NULL));
}

void no_debounce_app(const int button_pin) {
    init(button_pin);

    int prev_counter = 0;
    while(1) {
        if (prev_counter < counter) {
            ESP_LOGI("no-debounce", "Counter: %d", ++prev_counter);
        }

        //to free up some time for scheduler, othervise i get watchdog errors
        vTaskDelay(1);
    }
}