/*
 * serial.h
 *
 *  Created on: Aug 8, 2026
 *      Author: anisc
 */
#include "main.h"

#ifndef APPLICATION_USER_CORE_INC_SERIAL_H_
#define APPLICATION_USER_CORE_INC_SERIAL_H_

void print_to_serial(const char * msg, ...);
void print_raw_sensor_data(uint16_t temp, uint16_t light);
void print_processed_sensor_data(float temp, float light);

#endif /* APPLICATION_USER_CORE_INC_SERIAL_H_ */
