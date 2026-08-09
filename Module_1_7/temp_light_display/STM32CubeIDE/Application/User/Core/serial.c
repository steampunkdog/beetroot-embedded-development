#include "serial.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "main.h"

UART_HandleTypeDef *huart;

void serial_init(UART_HandleTypeDef *huart_n) {
	huart = huart_n;
}

void print_to_serial(const char * msg, ...) {
	static uint8_t outputBuffer[100] = {0};

	va_list args;
	va_start(args, msg);
	vsnprintf(outputBuffer, 100, msg, args);
    va_end(args);

    //CDC_Transmit_FS(outputBuffer, strlen(outputBuffer));
    HAL_UART_Transmit(huart, (uint8_t *)outputBuffer, strlen(outputBuffer), HAL_MAX_DELAY);
}

void print_raw_sensor_data(uint16_t temp, uint16_t light) {
	print_to_serial("RAW. Temp sensor: %d; Light sensor: %d\r\n", temp, light);
}

// NOTE -u _printf_float should be enabled for proper logs
void print_processed_sensor_data(float* values) {
	print_to_serial("Temperature: %4.1fC; Light: %.2elx\r\n", values[TEMPERATURE_IDX], values[ILLUMINANCE_IDX]);
}
