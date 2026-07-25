#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PIN 15
#define EXTERNAL_BUTTON_PIN 16

void blink_init() {
    //zero-initialize the config structure.
    gpio_config_t io_conf = {
        //set as output mode
        .mode = GPIO_MODE_OUTPUT,
        //bit mask of the pins that you want to set
        .pin_bit_mask = 1 << LED_PIN
    };
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = 1 << EXTERNAL_BUTTON_PIN;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void blink() {
    if (gpio_get_level(EXTERNAL_BUTTON_PIN)) {
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));

        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main() {
    blink_init();
    while(1) {
        blink();
    };
}