
#ifndef TRAFFIC_LIGHTS_COMMANDS_H
#define TRAFFIC_LIGHTS_COMMANDS_H

#include <stdint.h>

typedef enum {
	COMMAND_NONE = 0,
	COMMAND_RUN = 1,
	COMMAND_STOP = 2,
	COMMAND_FLASHING_YELLOW = 3,
	COMMAND_CONFIG = 4
} command_types;

#define NUMBER_OF_CONFIG_PARAMS 8
typedef struct {
	uint16_t green_ms;
	uint16_t green_flashing_ms;
	uint16_t green_flashing_period;
	uint16_t yellow_ms;
	uint16_t red_ms;
	uint16_t red_and_yellow_ms;

	uint16_t flashing_yellow_period;
} traffic_config_t;

typedef struct {
	command_types command;
	traffic_config_t *config;
} command_t;

command_t* alloc_and_create_command(command_types command, traffic_config_t *config);
traffic_config_t* alloc_and_create_config();
void free_command(command_t* command);

#endif
