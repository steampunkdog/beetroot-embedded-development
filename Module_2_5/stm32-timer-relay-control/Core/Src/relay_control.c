#include "relay_control.h"
#include "logger.h"

static TIM_HandleTypeDef htim2;
static relay_config_t relay_config;

static void set_state(GPIO_PinState new_state);

void relay_control_init(relay_config_t r_config) {
    relay_config = r_config;

    uint16_t full_on_off_period = relay_config.activity_time_s + relay_config.inactivity_time_s;
    uint16_t state_switch_cnt = relay_config.start_active ? relay_config.activity_time_s : relay_config.inactivity_time_s;

    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2 = (TIM_HandleTypeDef){
        .Instance = TIM2,
        // ABP1 and it's prescalers configured to provide 16MHz
        // to achive 1 tick per ms 16MHz/16000 = 1000 ticks/s
        .Init.Prescaler = 15999,
        .Init.CounterMode = TIM_COUNTERMODE_UP,
        .Init.Period = full_on_off_period * 1000 - 1,
        .Init.ClockDivision = TIM_CLOCKDIVISION_DIV1,
        htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE
    };

    HAL_StatusTypeDef status;
    status = HAL_TIM_Base_Init(&htim2);
    log_to_serial("Base init: %d\r\n", status);

    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    TIM_ClockConfigTypeDef clock_source_config = {
        .ClockSource = TIM_CLOCKSOURCE_INTERNAL
    };

    status = HAL_TIM_ConfigClockSource(&htim2, &clock_source_config);
    log_to_serial("Clock config: %d\r\n", status);

    TIM_OC_InitTypeDef clock_oc_config = {
        .OCMode = TIM_OCMODE_TIMING,
        .Pulse = state_switch_cnt * 1000 -1 ,
        .OCPolarity = TIM_OCPOLARITY_HIGH,
        .OCFastMode = TIM_OCFAST_DISABLE
    };

    status = HAL_TIM_OC_ConfigChannel(&htim2, &clock_oc_config, TIM_CHANNEL_4);
    log_to_serial("OC config: %d\r\n", status);

    status = HAL_TIM_Base_Start_IT(&htim2);
    log_to_serial("Base start: %d\r\n", status);
    status = HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_4);
    log_to_serial("OC start: %d\r\n", status);

    //set inital state
    set_state(relay_config.start_active);
}

void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        set_state(!relay_config.start_active);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        set_state(relay_config.start_active);
    }
}

static void set_state(GPIO_PinState new_state) {
    HAL_GPIO_WritePin(relay_config.port, relay_config.relay_pin, new_state);
            
    HAL_GPIO_WritePin(relay_config.port, relay_config.on_led_pin, new_state);
    HAL_GPIO_WritePin(relay_config.port, relay_config.off_led_pin, !new_state);

    log_to_serial("Relay is now %s\r\n", new_state ? "OPEN" : "CLOSED");
}


