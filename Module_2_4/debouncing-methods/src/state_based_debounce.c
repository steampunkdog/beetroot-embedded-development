#include "driver/gpio.h"
#include <esp_timer.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static volatile int counter = 0;
static TaskHandle_t counter_task_handle;
static int button_pin;


static void button_press_handler(void *args) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(counter_task_handle, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

static void counter_task(void *args) {
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(20));

        if (gpio_get_level(button_pin) == 0) {
            ESP_LOGI("state-based-debounce", "Counter: %d", ++counter);
        }
    }
} 

void init() {
    gpio_config_t button_conf = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask = (1 << button_pin)
    };

    ESP_ERROR_CHECK(gpio_config(&button_conf));

    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    ESP_ERROR_CHECK(gpio_isr_handler_add(button_pin, button_press_handler, NULL));

    xTaskCreatePinnedToCore(
        counter_task,
        "counter",
        2048,
        NULL,
        5,
        &counter_task_handle,
        1
    );
}

void state_based_debounce_app(const int b_pin) {
    button_pin = b_pin;
    init();
}