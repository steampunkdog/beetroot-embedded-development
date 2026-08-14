#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include <esp_timer.h>
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_cmpr.h"

#define MAIN_TAG "main"

#define POTENTIOMETER_CHANNEL ADC_CHANNEL_0 // GPIO 1
#define PWM_OUTPUT_PIN 38

#define PWM_CLOCK_RESOLUZTION_HZ 1'600'000
#define PWM_FREQUENCY 25'000

adc_oneshot_unit_handle_t adc_handle;
mcpwm_cmpr_handle_t pwm_comparator_handle;

void adc_init();
void pwm_init();

// Calculates a tick, that will trigger LOW signal for raw ADC value
// using division it basically maps [0; 4095] range of ADC values onto [0; 63] range of ticks
int first_low_tick_from_raw_adc_value(int raw) {
    #define N_OF_TICKS 64
    return raw / N_OF_TICKS;
}

void app_main() {
    adc_init();
    pwm_init();

    int raw = 0;
    while (1) {
        
        esp_err_t adc_error = adc_oneshot_read(adc_handle, POTENTIOMETER_CHANNEL, &raw);
        if (adc_error) {
            ESP_LOGE(MAIN_TAG, "ADC Error code: %d", adc_error);
            // 10s delay in case of error to read error and stop controller if needed
            vTaskDelay(pdMS_TO_TICKS(10000)); 
            continue;
        }

        int tick_value = first_low_tick_from_raw_adc_value(raw);
        ESP_LOGI(MAIN_TAG, "ADC raw value: %4d; First low tick: %2d", raw, tick_value);
        esp_err_t pwm_error = mcpwm_comparator_set_compare_value(pwm_comparator_handle, tick_value);
        if (pwm_error)  {
            ESP_LOGE(MAIN_TAG, "PWM Error code: %d", pwm_error);
            // 10s delay in case of error to read error and stop controller if needed
            vTaskDelay(pdMS_TO_TICKS(10000)); 
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void adc_init() {
    adc_oneshot_unit_init_cfg_t adc_conf = {
        .unit_id = ADC_UNIT_1,
        .clk_src = 0,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_conf, &adc_handle));

    adc_oneshot_chan_cfg_t adc_chan_conf = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, POTENTIOMETER_CHANNEL, &adc_chan_conf));
}

void pwm_init() {
    mcpwm_timer_handle_t pwm_timer_handle;
    mcpwm_oper_handle_t pwm_operator_handle;
    mcpwm_gen_handle_t pwm_generator_handle;

    // Configure timer for 1.6MHz, with 64 ticks per period
    // this gives us 25kHz PWM
    mcpwm_timer_config_t pwm_timer_conf = {
        .intr_priority = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = PWM_CLOCK_RESOLUZTION_HZ, // clock ticks per second
        .period_ticks = PWM_CLOCK_RESOLUZTION_HZ / PWM_FREQUENCY, // ticks per period
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };

    ESP_ERROR_CHECK(mcpwm_new_timer(&pwm_timer_conf, &pwm_timer_handle));
    ESP_ERROR_CHECK(mcpwm_timer_enable(pwm_timer_handle));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(pwm_timer_handle, MCPWM_TIMER_START_NO_STOP));

    mcpwm_operator_config_t pwm_operator_conf = {
        .group_id = 0,
        .intr_priority = 0
    };

    ESP_ERROR_CHECK(mcpwm_new_operator(&pwm_operator_conf, &pwm_operator_handle));

    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(pwm_operator_handle, pwm_timer_handle));

    // Comporator is used to set a tick number that will trigger an event
    // Action configured lower will react to this event and change output level
    mcpwm_comparator_config_t pwm_comparator_config = {
        .intr_priority = 0
    };

    ESP_ERROR_CHECK(mcpwm_new_comparator(pwm_operator_handle, &pwm_comparator_config, &pwm_comparator_handle));

    // Generator controls signal level on pin
    mcpwm_generator_config_t pwm_generator_conf = {
        .gen_gpio_num = PWM_OUTPUT_PIN,
    };

    ESP_ERROR_CHECK(mcpwm_new_generator(pwm_operator_handle, &pwm_generator_conf, &pwm_generator_handle));

    // This action sets PWM pin level HIGH each time the new tick cycle starts
    ESP_ERROR_CHECK(
        mcpwm_generator_set_action_on_timer_event(
            pwm_generator_handle,
            MCPWM_GEN_TIMER_EVENT_ACTION(
                // Register the event only when counting up
                // MCPWM_TIMER_COUNT_MODE_UP - so, basically, any count
                MCPWM_TIMER_DIRECTION_UP,
                // If the tick counter got reset. 
                // Happens every n=pwm_timer_conf.period_ticks ticks
                MCPWM_TIMER_EVENT_EMPTY,
                // Set GPIO level as HIGH
                MCPWM_GEN_ACTION_HIGH
            )
        )
    );

    // This action sets PWM pin level LOW each time 
    // the tick number == compare value set with compartor
    ESP_ERROR_CHECK(
        mcpwm_generator_set_action_on_compare_event(
            pwm_generator_handle,
            MCPWM_GEN_COMPARE_EVENT_ACTION(
                // Register the event only when counting up
                MCPWM_TIMER_DIRECTION_UP,
                // If comparator condition is met
                pwm_comparator_handle,
                // Set GPIO level LOW
                MCPWM_GEN_ACTION_LOW
            )
        )
    );
}