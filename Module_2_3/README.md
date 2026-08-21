# Модуль 2.3 Анищенко М.О. 

## Хід роботи

ВІДЕО РОБОТИ: [https://youtube.com/shorts/eu_eHo6d22M](https://youtube.com/shorts/eu_eHo6d22M)
ПРОЕКТ: [three-led-blink-without-delay/src/main.c](three-led-blink-without-delay/src/main.c)

### Код

`blinking_led` - структура що зберігає конфігураційні та ран-тайм параметри світлодіода.
``` c
// Describes blinking LED
typedef struct{
    const gpio_num_t pin;                 // GPIO PIN
    const uint32_t blink_period_micros;   // How often does LED change state in microseconds
    uint32_t level;                       // Current signal level
    uint32_t number_of_blinks;            // How many times have led blinked
} blinking_led;
```

Для зручності `blinking_led` для всіх задіяних світлодіодів зберігаються у масиві
``` c
// Array of all blinking LEDs
blinking_led leds[3] = {
    {RED_LED_PIN, RED_LED_BLINK_PERIOD*1000, 0, 0},
    {GREEN_LED_PIN, GREEN_LED_BLINK_PERIOD*1000, 0, 0},
    {BLUE_LED_PIN, BLUE_LED_BLINK_PERIOD*1000, 0, 0}
};
```

Перед початком циклу записується мітка часу `started`.
На початку кожної ітерації записуються час її початку `now`. 
При обході массиву світлодіодів для кожного з них розраховується кількість перемикань що мали відбутися у період між `started` та `now`.
Якщо обрахована кількість перемикань > фактичної кількості - відбувається переминання стану і нове значення кількості записується в відповідну структуру.
``` c
void app_main() {
    init();

    uint64_t started = esp_timer_get_time();
    while (1) {
        uint64_t now = esp_timer_get_time();

        // For each LED calculate how much time should it have blinked at the current timestamp
        // if it it more that the actual number of blinks that occured - blink.
        // Using this way, instead of just comparing `now` timestamp to previous blink timestamp,
        // prevents blinking "drift" and blink timing error accumulation.
        for (int i = 0; i < number_of_leds; i++) {
            blinking_led *led = &leds[i];
            if ((now - started)/led->blink_period_micros > led->number_of_blinks) {
                gpio_set_level(led->pin, ~led->level);
                led->level = ~led->level;
                led->number_of_blinks++;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```