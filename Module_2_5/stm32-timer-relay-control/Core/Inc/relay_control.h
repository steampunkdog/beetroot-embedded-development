#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "stm32f4xx_hal_tim_ex.h"

typedef struct {
    GPIO_TypeDef* port;
    uint16_t relay_pin;
    uint16_t on_led_pin;
    uint16_t off_led_pin;

    uint16_t inactivity_time_s;
    uint16_t activity_time_s;
    uint16_t start_active;
} relay_config_t;


void relay_control_init(relay_config_t r_config);


#endif
