#include "main.h"

#ifndef APPLICATION_USER_CORE_INC_SENSOR_H_
#define APPLICATION_USER_CORE_INC_SENSOR_H_

typedef enum {
	TEMPERATURE = 0,
	LIGHT = 1
} sensor_to_index_t;

void get_calculated_sensor_values(uint16_t* raw, uint16_t* output);

#endif /* APPLICATION_USER_CORE_INC_SENSOR_H_ */
