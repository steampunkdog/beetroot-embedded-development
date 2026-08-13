#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include <esp_timer.h>
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#define INIT_TAG "init"
#define ADC_TAG "adc"
#define POTENTIOMETER_CHANNEL ADC_CHANNEL_0

adc_oneshot_unit_handle_t adc_handle;

void init() {
    adc_oneshot_unit_init_cfg_t adc_conf = {
        .unit_id = ADC_UNIT_1,
        .clk_src = 0,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_conf, &adc_handle));

    adc_oneshot_chan_cfg_t chan_conf = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, POTENTIOMETER_CHANNEL, &chan_conf));
}

void app_main() {
    init();
    int raw = 0;
    while (1) {
        esp_err_t adc_error = adc_oneshot_read(adc_handle, POTENTIOMETER_CHANNEL, &raw);
        if (adc_error) {
            ESP_LOGE(ADC_TAG, "ADC Error code: %d", adc_error);
            continue;
        }
        ESP_LOGI(ADC_TAG, "ADC raw value: %d", raw);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
}