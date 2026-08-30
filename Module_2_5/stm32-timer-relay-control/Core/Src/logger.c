#include <logger.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define OUTPUT_BUFF_SIZE 200
static UART_HandleTypeDef *huart;

void logger_init(UART_HandleTypeDef *huart_n) {
	huart = huart_n;
}

void log_to_serial(const char * msg, ...) {
	static char outputBuffer[OUTPUT_BUFF_SIZE] = {0};

	va_list args;
	va_start(args, msg);
	vsnprintf(outputBuffer, (size_t)OUTPUT_BUFF_SIZE, msg, args);
    va_end(args);

    HAL_UART_Transmit(huart, (uint8_t *)outputBuffer, (uint16_t)strlen(outputBuffer), HAL_MAX_DELAY);
}
