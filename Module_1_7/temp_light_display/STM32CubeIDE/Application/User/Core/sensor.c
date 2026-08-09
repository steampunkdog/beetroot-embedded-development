#include "sensor.h"

#include "main.h"
#include "serial.h"
#include <math.h>

#define TEMP_PULLDOWN_R 10000
#define LIGHT_PULLDOWN_R 1000
#define QUANTIZATION 4095
#define VCC_VOLTAGE 3.3f

static float calculate_temperature(uint16_t raw);
static float calculate_illuminance(uint16_t raw);

void get_calculated_sensor_values(uint16_t* raw, uint16_t* output) {
	uint16_t temp_raw = raw[TEMPERATURE_IDX];
	uint16_t illum_raw = raw[ILLUMUNANCE_IDX];
	print_raw_sensor_data(temp_raw, illum_raw);

	output[TEMPERATURE_IDX] = calculate_temperature(temp_raw);
	output[ILLUMUNANCE_IDX] = calculate_illuminance(illum_raw);
}

static float calculate_temperature (uint16_t raw) {
	//TODO double-check coefficients https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
	#define SHC_A 1.785753086e-3f
	#define SHC_B 1.242576425e-4f
	#define SHC_C 5.424282270e-7f

//	print_to_serial("A: %f; B: %f; C: %f\r\n", SHC_A, SHC_B, SHC_C);
	float r = TEMP_PULLDOWN_R * ((float)QUANTIZATION/raw - 1);
//	print_to_serial("TEMP R: %f\r\n", r);
	float t_K = 1.0 / (SHC_A + SHC_B*log(r) + SHC_C*pow(log(r), 3.0));
//	print_to_serial("TEMP t_K: %f\r\n", t_K);
	return t_K - 273.15;

}

#define LDR_SENSITIVITY 0.5f
#define LDR_A 15811

static float calculate_illuminance(uint16_t raw) {
	float r = LIGHT_PULLDOWN_R * ((float)QUANTIZATION/raw - 1);
//	print_to_serial("ILLUM R: %f\r\n", r);
	return pow(LDR_A/r, 1/LDR_SENSITIVITY);
}



