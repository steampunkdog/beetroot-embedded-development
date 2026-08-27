#ifndef TRAFFIC_LIGHTS_H
#define TRAFFIC_LIGHTS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "stm32f4xx_hal.h"

typedef struct {
	GPIO_TypeDef* gpio_port;
	uint16_t green_light_pin;
	uint16_t yellow_light_pin;
	uint16_t red_light_pin;
} pins_config_t;

void traffic_lights_init(QueueHandle_t* command_queue_h, pins_config_t *config);

#endif
