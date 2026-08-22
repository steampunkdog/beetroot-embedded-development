#include "no_debounce.h"
#include "time_based_debounce.h"
#include "state_based_debounce.h"
#include "pooling_debounce.h"

#define BUTTON_PIN 15

void app_main() {
    time_based_debounce_app(BUTTON_PIN);
}