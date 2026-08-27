#ifndef APPLICATION_USER_CORE_INC_SERIAL_H_
#define APPLICATION_USER_CORE_INC_SERIAL_H_

#include "stm32f4xx_hal.h"

void logger_init(UART_HandleTypeDef *huart_n);
void log_to_serial(const char * msg, ...);

#endif
