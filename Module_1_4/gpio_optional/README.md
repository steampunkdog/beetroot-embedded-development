# gpio_optional

Проект що додаткове додаткове завдання Модуля 1.4. \
Два світлодіоди (червоний та зелений) блимають в одному з двох режимів:
* SERIAL - світлодіоди блимають почергово
* SIMULTANEOUS - світлодіоди блимають одночасно

Чотири швидкості мерехтіння: 1000мс, 500мс, 250мс, 125мс.
Дві кнопки керують швидкістю та режимом:
* BOOT - наступна швидкість
* Зовнішня кнопка - попередня швидкість
* Одночасне натискання обох - перемикання режиму

## Плата

- `esp32-s3-devkitc-1-n16r8v`
- Середовище PlatformIO: `esp32-s3-devkitc-1-n16r8v`

## Підключені до плати пристрої

- Червоний світлодіод підключено як вихід GPIO `15`
- Зелений світлодіод підключено як вихід GPIO `16`
- Кнопка "попередній" підключена як вхід GPIO `17`
- Кнопка "наступний" (BOOT) підключена як вхід GPIO `0`

## Схема

![Схема](misc/scheme.png)

Зовнішня кнопка, що була в комплекті, вже має вбудований резистор на піні підписаному **GND**, тож немає необхідності в додатковому "підтягуючому резисторі".
Також, оскільки я хотів отримати однакову логіку обробки для обох кнопок(**BOOT** пропускає струм в неактивному стані), я підключив зовнішня кнопку в "зворотньому напрямку":
* **GND** кнопки до **3V3** плати
* **3V3** кнопки до **GND** плати

## Робота

ВІДЕО РОБОТИ: [https://youtu.be/5AMlNgHUF-8](https://youtu.be/5AMlNgHUF-8)

### Ітерація між швидкостями мерехтіння

Підтримується 4 швидкості мерехтіння, що зберігаються в масиві:
```c
#define NUMBER_OF_SPEED_MODES 4
const int blink_speeds[NUMBER_OF_SPEED_MODES] = {1000, 500, 250, 125};
volatile int current_blink_speed_idx = 0;
```
При натисканні кнопки **NEXT** індекс збільшується на 1 з циклічним перекриттям.
При натисканні кнопки **PREV** індекс зменшується на 1 з циклічним перекриттям.

### Реєстрація одночасного натискання кнопок та перемикання режимів

Обробники переривань оновлюють часові мітки останнього натискання.
Основна логіка виконується у окремій задачі `input_controller` з пріоритетом 4.
Задача преревіряє час останнього натискання кнопки записаний у змінній кожні 200мс, записує стан(натиснута чи ні) у відповідний біт змінної **state** та використовує бітову маску для визначення стану(1 біт - BOOT, 2 - зовнішня). Можливі стани:
* `0` - жодна кнопка не натиснута
* `1` - натиснута тільки кнопка NEXT (наступна швидкість)
* `2` - натиснута тільки кнопка PREV (попередня швидкість)
* `3` - натиснуті обидві кнопки (перемикання режиму)

```c
state = 0;
if (last_next_triggered_time > last_controller_run && last_next_triggered_time < now) {
    state += (1 << 0);
}
if (last_prev_triggered_time > last_controller_run && last_prev_triggered_time < now) {
    state += (1 << 1);
}
```

### Логування

Використовується API логування ESP-IDF (`esp_log.h`), що записує логи у Serial
Два теги для різних компонентів системи:
* `main` - для повідомлень про ініціалізацію
* `controller` - для повідомлень про зміну стану

При зміні швидкості або режиму логується відповідне повідомлення:
```c
ESP_LOGI(CONTROLLER_TAG, "Got 'NEXT' signal. Switching to speed %d", current_blink_speed_idx);
ESP_LOGI(CONTROLLER_TAG, "Got 'MODE' signal. Switching to mode %d", current_blink_mode);
```

Приклад логів
```
I (8282) controller: Got 'PREV' signal. Switching to speed 3
I (13082) controller: Got 'NEXT' signal. Switching to speed 0
I (15682) controller: Got 'NEXT' signal. Switching to speed 1
I (17282) controller: Got 'PREV' signal. Switching to speed 0
I (17882) controller: Got 'PREV' signal. Switching to speed 3
I (18282) controller: Got 'PREV' signal. Switching to speed 2
I (19082) controller: Got 'MODE' signal. Switching to mode 1
I (21082) controller: Got 'NEXT' signal. Switching to speed 3
I (22082) controller: Got 'NEXT' signal. Switching to speed 0
I (23082) controller: Got 'MODE' signal. Switching to mode 0
I (25082) controller: Got 'PREV' signal. Switching to speed 3
```