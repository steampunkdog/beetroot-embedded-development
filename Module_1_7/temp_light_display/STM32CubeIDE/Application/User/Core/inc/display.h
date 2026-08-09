/*
 * display.h
 *
 *  Created on: Aug 9, 2026
 *      Author: myan
 */
#include "main.h"

#ifndef APPLICATION_USER_CORE_INC_DISPLAY_H_
#define APPLICATION_USER_CORE_INC_DISPLAY_H_

uint8_t display_init(I2C_HandleTypeDef *hi2c_n);
void draw_sensor_values(float* values);

#endif /* APPLICATION_USER_CORE_INC_DISPLAY_H_ */
