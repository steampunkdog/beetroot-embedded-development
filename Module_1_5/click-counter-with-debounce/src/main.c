#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include <esp_timer.h>
#include "esp_log.h"

#define DEFAULT_DEBOUNCE_MS 5 // default debounce in ms. This value can be ignored if controller's tickrate is lover then 200Hz
#define BUTTON_PIN 15
#define COUNTER_TAG "counter"
#define INIT_TAG "init"

static int DEBOUNCE_TICKS = 0; // debounce in ticks. Calculated upon init

static TaskHandle_t counter_task_handle = NULL;

void button_isr_handler(void *args) {
    // disable interrupt to prevent additional button presses before counter handles current one
    gpio_intr_disable(BUTTON_PIN); 

    // notify counter task
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(counter_task_handle, &xHigherPriorityTaskWoken);

    // check if woken task has higher priority, if so - immediately switch to it
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void counter_task(void *params) {
    static uint8_t counter = 0;
    while(1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // wait for notification from interrupt
        vTaskDelay(DEBOUNCE_TICKS);  // wait for debounce delay

        // increment counter only if button is still pressed after debounce period
        if(gpio_get_level(BUTTON_PIN) == 0) {
            counter++;
            ESP_LOGI(COUNTER_TAG, "Button was pressed. Counter: %2d; Click time: %d", counter, esp_timer_get_time());
        }

        gpio_intr_enable(BUTTON_PIN);
    }
}

void init() {

    // if tickrate is to low to get 5ms debounce - set debounce of 1 tick
    DEBOUNCE_TICKS = pdMS_TO_TICKS(DEFAULT_DEBOUNCE_MS);
    if (DEBOUNCE_TICKS == 0) {
        DEBOUNCE_TICKS = 1;
    }
    ESP_LOGI(INIT_TAG, "Debounce delay: %dms", DEBOUNCE_TICKS*1000/configTICK_RATE_HZ);

    gpio_config_t button = {
        .pin_bit_mask = 1 << BUTTON_PIN,
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE // pullup is built into button
    };
    gpio_config(&button);

    xTaskCreatePinnedToCore(
        counter_task,
        "counter_task",
        2048,
        NULL,
        10,
        &counter_task_handle,
        1
    );

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

}

void app_main() {
    init();
}