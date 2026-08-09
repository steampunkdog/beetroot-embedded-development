#include "display.h"

#include <stdio.h>
#include "main.h"
#include "fonts.h"
#include "ssd1306.h"

#define BACKGROUND_COLOR Black
#define TEXT_COLOR White

static void draw_divider();
static void draw_temperature(float temp);
static void draw_illumunance(float illum);
static void clear_screen();
static void update_screen();

static I2C_HandleTypeDef *hi2c;

uint8_t display_init(I2C_HandleTypeDef *hi2c_n) {
	uint8_t status = ssd1306_Init(hi2c_n);
	if (!status) {
		hi2c = hi2c_n;
	}
	return status;
}

void draw_sensor_values(float* values) {
	clear_screen();
	draw_temperature(values[TEMPERATURE_IDX]);
	draw_divider();
	draw_illumunance(values[ILLUMINANCE_IDX]);
	update_screen();
}

static void draw_divider() {
	#define DIVIDER_START_X 0
	#define DIVIDER_START_Y 30
	#define DIVIDER_END_X 125
	#define DIVIDER_END_Y 34

	for (int x = DIVIDER_START_X; x <= DIVIDER_END_X; x++) {
		for (int y = DIVIDER_START_Y; y < DIVIDER_END_Y; y++) {
			ssd1306_DrawPixel(x, y, TEXT_COLOR);
		}
	}
}

static void draw_temperature(float temp) {
	#define TEMP_TEXT_START_X 26
	#define TEMP_TEXT_START_Y 6
	static char buf[7];

	ssd1306_SetCursor(TEMP_TEXT_START_X, TEMP_TEXT_START_Y);
	sprintf(buf, "%+3.1f C", temp);
	ssd1306_WriteString(buf, Font_11x18, TEXT_COLOR);
}

static void draw_illumunance(float illum) {
	#define ILLUM_TEXT_START_X 15
	#define ILLUM_TEXT_START_Y 40
	static char buf[9];

	ssd1306_SetCursor(ILLUM_TEXT_START_X, ILLUM_TEXT_START_Y);
	sprintf(buf, "%.1e lx", illum);
	ssd1306_WriteString(buf, Font_11x18, TEXT_COLOR);
}

static void clear_screen() {
	ssd1306_Fill(BACKGROUND_COLOR);
}

static void update_screen() {
	ssd1306_UpdateScreen(hi2c);
}
