# ДЗ Модуль 2.2; Анищенко М.О.

## Основне завдання

ВІДЕО РОБОТИ: [https://youtube.com/shorts/SYNLRO0d6cI?feature=share](https://youtube.com/shorts/SYNLRO0d6cI?feature=share) \
ПРОЕКТ: [relay-rw/src/main.c](relay-rw/src/main.c)

### Схема

*Виходи:* до піна **16** підключено базу транзистора **Q1**, до еммітера транзистора підключено вхід реле **K1**. Також, для убезпечення піна від струму розряду котушки, параллельно до еммітра та землі, в зворотньому напрямку(анод до землі), підключено flyback-діод **D1**. \
*Входи:* **Normaly-Closed** вихід реле **K1** підключено до піна **15** з застосуванням **pull-down** резистора **R2**

![Схема](relay-rw/media/scheme.png)

### Код

**Запис міток часу:** \
Для запису часу зміни значення на виході реле використано переривання в режимі **GPIO_INTR_ANYEDGE**. Функція **handle_relay_input** обробляє переривання, воно записує мітку часу в момент зміни значення сигналу у массив **isr_triggered_timestamps**. Массив було використано бо реле схильне до брязкоту контактів, і запис лише одного значення не давав би повноти інформації. Також це дає можливість виміряти тривалість брязкоту контактів. \
Після кожної ітерації вимірювання, массив та вказівник на поточний індекс в ньому очищається функцією **clear_timestamps**

``` c
static volatile int64_t isr_triggered_timestamps[MAX_STORED_TIMESTAMPS] = {};
static volatile size_t writer_index = 0;
static volatile bool overflow = false;

void handle_relay_input(void *arg) {
    if (writer_index == MAX_STORED_TIMESTAMPS) {
        overflow = true;
        return;
    }
    isr_triggered_timestamps[writer_index] = esp_timer_get_time();
    writer_index++;
}

void clear_timestamps() {
    for (size_t i = 0; i < MAX_STORED_TIMESTAMPS; i++) {
        isr_triggered_timestamps[i] = 0;
    }
    writer_index = 0;
    overflow = false;
}
```

**В основному циклі програми:**
1. Інвертується сигнал що подається на пін
2. Одразу після зміни сигналу - записується мітка часу
3. Для запису усіх спрацювань переривання - контроллер очікує **COLLECT_RESULTS_DELAY**(1с)
4. В циклі відбувається прохід по всім записаним міткам часу записаним перериванням
    1. Якщо це перша мітка - виводиться різниця між нею і міткою з **п.2**. Це час від встановлення значення сигналу до першої події на виході реле
    2. Для кожного індексу виводиться індекс і мітка часу
    3. Якщо це остання мітка - виводиться різниця між нею і міткою з **п.2**. Це час від встановлення значення сигналу до досягнення стабільного значення сигналу на виході реле
5. Якщо переривання спацювало більше разів, ніж можливо було записати в масив - виводиться відповідне попередження
6. Вміст масиву та індекс очищається функцією **clear_timestamps**
7. Значення змінної, що зберігає значення піна, інвертується для наступної ітерації
8. Для зручності, викликається додаткова затримка між ітераціями вимірів

``` c
void app_main() {
    init();
    int iteration = 1;
    while (1) {
        static int set_pin_to = 1;

        ESP_LOGI(MEASUREMENTS_TAG, "-----------------------------------------");
        ESP_LOGI(MEASUREMENTS_TAG, "Iteration %d", iteration);

        gpio_set_level(OUTPUT_PIN, set_pin_to);
        uint64_t pin_set_timestamp = esp_timer_get_time();
        ESP_LOGI(MEASUREMENTS_TAG, "Pin level set to %d, at %lld. Collecting input...", set_pin_to, pin_set_timestamp);

        vTaskDelay(pdMS_TO_TICKS(COLLECT_RESULTS_DELAY));

        for (size_t i = 0; i < writer_index; i++) {
            if(i == 0) {
                ESP_LOGI(MEASUREMENTS_TAG, "Time to first switch: %lld", isr_triggered_timestamps[i] - pin_set_timestamp);
            }

            ESP_LOGI(MEASUREMENTS_TAG, "State changed i: %zu at: %lld", i, isr_triggered_timestamps[i]);

            if(i + 1 == writer_index) {
                ESP_LOGI(MEASUREMENTS_TAG, "Time to stable state: %lld", isr_triggered_timestamps[i] - pin_set_timestamp);
            }
        }

        if (overflow) {
            ESP_LOGW(MEASUREMENTS_TAG, "Got overflow");
        }
        
        clear_timestamps();
        set_pin_to = !set_pin_to;

        vTaskDelay(pdMS_TO_TICKS(MEASUREMENTS_DELAY));
        iteration++;
    }
}
```

### Лог

**Приклад логу:**
```
I (281) measurement: -----------------------------------------
I (281) measurement: Iteration 1
I (281) measurement: Pin level set to 1, at 61168. Collecting input...
I (1291) measurement: Time to first switch: 7621
I (1291) measurement: State changed i: 0 at: 68789
I (1291) measurement: State changed i: 1 at: 68807
I (1291) measurement: State changed i: 2 at: 68823
I (1291) measurement: State changed i: 3 at: 68880
I (1301) measurement: State changed i: 4 at: 68923 
I (1301) measurement: State changed i: 5 at: 69005
I (1311) measurement: State changed i: 6 at: 69083
I (1311) measurement: State changed i: 7 at: 69102
I (1311) measurement: State changed i: 8 at: 69117
I (1321) measurement: State changed i: 9 at: 70467
I (1321) measurement: State changed i: 10 at: 70854
I (1331) measurement: Time to stable state: 9686
I (2331) measurement: -----------------------------------------
I (2331) measurement: Iteration 2
I (2331) measurement: Pin level set to 0, at 2104968. Collecting input...
I (3331) measurement: Time to first switch: 1116
I (3331) measurement: State changed i: 0 at: 2106084
I (3331) measurement: Time to stable state: 1116
I (4331) measurement: -----------------------------------------
```

**Повний лог:** [relay-rw/media/log.txt](relay-rw/media/log.txt)

### Результати замірів

| Перехід стану реле | Час до першого перемикання | Час брязкоту |
| ------------------ | -------------------------- | ------------ |
| Off -> On          | 7.624 мс                   | 2.07 мс      |
| On -> Off          | 1.108 мс                   | 0 мс         |

Середній час **Off -> On** переходу реле, до першого замикання ключа - **7.624 мс**, з тривалістю брязкоту в 2.07 мс \
Середній час **On -> Off** переходу реле, до першого розмикання ключа, значно швидший - **1.108 мс**, а брязкіт при такому переході - відсутній

## Додаткове завдання

ВІДЕО РОБОТИ: \
ПРОЕКТ: [dc-motor-control/src/main.c](dc-motor-control/src/main.c)