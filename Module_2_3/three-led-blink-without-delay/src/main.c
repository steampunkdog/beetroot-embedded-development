#include <stdlib.h>
#include <stdio.h>
#include <esp_timer.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define RED_LED_PIN 15
#define RED_LED_BLINK_PERIOD 200

#define GREEN_LED_PIN 16
#define GREEN_LED_BLINK_PERIOD 500

#define BLUE_LED_PIN 17
#define BLUE_LED_BLINK_PERIOD 1000

// Describes blinking LED
typedef struct{
    const gpio_num_t pin;                 // GPIO PIN
    const uint32_t blink_period_micros;   // How often does LED change state in microseconds
    uint32_t level;                       // Current signal level
    uint32_t number_of_blinks;            // How many times have led blinked
} blinking_led;

// Array of all blinking LEDs
blinking_led leds[3] = {
    {RED_LED_PIN, RED_LED_BLINK_PERIOD*1000, 0, 0},
    {GREEN_LED_PIN, GREEN_LED_BLINK_PERIOD*1000, 0, 0},
    {BLUE_LED_PIN, BLUE_LED_BLINK_PERIOD*1000, 0, 0}
};

// Number of leds
const int number_of_leds = sizeof(leds) / sizeof(blinking_led);

void init() {
    gpio_config_t led_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1 << RED_LED_PIN) | (1 << GREEN_LED_PIN) | (1 << BLUE_LED_PIN)
    };
    gpio_config(&led_conf);
}


void app_main() {
    init();

    uint64_t started = esp_timer_get_time();
    while (1) {
        uint64_t now = esp_timer_get_time();

        // For each LED calculate how much time should it have blinked at the current timestamp
        // if it it more that the actual number of blinks that occured - blink.
        // Using this way, instead of just comparing `now` timestamp to previous blink timestamp,
        // prevents blinking "drift" and blink timing error accumulation.
        for (int i = 0; i < number_of_leds; i++) {
            blinking_led *led = &leds[i];
            if ((now - started)/led->blink_period_micros > led->number_of_blinks) {
                gpio_set_level(led->pin, ~led->level);
                led->level = ~led->level;
                led->number_of_blinks++;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}