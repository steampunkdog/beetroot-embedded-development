#ifndef TRAFFIC_LIGHTS_COMMAND_HANDLING_H
#define TRAFFIC_LIGHTS_COMMAND_HANDLING_H

#include "FreeRTOS.h"
#include "queue.h"
#include "stm32f4xx_hal.h"

void init_command_handler(UART_HandleTypeDef *huart, QueueHandle_t command_queue);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

#endif
