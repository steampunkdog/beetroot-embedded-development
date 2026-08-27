#include "commands.h"
#include "FreeRTOS.h"

command_t* alloc_and_create_command(command_types command, traffic_config_t *config) {
	command_t* new_command = pvPortMalloc(sizeof(command_t));
	new_command->command = command;
	new_command->config = config;
	return new_command;

}

traffic_config_t* alloc_and_create_config() {
	return pvPortMalloc(sizeof(traffic_config_t));
}
void free_command(command_t* command) {
	if (command == NULL) {
		return;
	}

	if (command->config != NULL) {
		vPortFree(command->config);
	}
	vPortFree(command);
}
