# ДЗ Модуль 1.6; Анищенко М.О.

## Хід роботи

ВІДЕО РОБОТИ: [https://youtube.com/shorts/_pB_oeZOpnY?feature=share](https://youtube.com/shorts/_pB_oeZOpnY?feature=share)

Проект: [blink_without_delay](blink_without_delay)

**Примітка:** Цього разу виконував основне і додаткове завдання в одному проекті, бо вони доповнювали одне-одне.

### Код 

`Використати constexpr для номера піну та часу блимання` \
`Всі “магічні числа” повинні бути винесені у constexpr/static const` \
Згідно вимоги піни та інші статичні значення визначено як constexpr

``` c
#include <Arduino.h>

constexpr int LED_PIN = 15;
constexpr int BUTTON_PIN = 16;
constexpr int BLINK_PERIOD_MICROS = 300000; // 300ms
constexpr int DEBOUNCE_MICROS = 5000;       // 5ms
constexpr int NUMBER_OF_MODES = 3;
```

`Використати enum class для стану LED (On / Off)` \
Згідно вимоги **LedState** об'явлено як enum class. **LedMode** - використовується для додаткового завдання ,він перелічує режими світіння, для того щоб спростити логіку перемикання режимів **LedMode** визначено як звичайний **enum**, це дозволяє працювати з ним як з **int**
``` c
enum class LedState {
  OFF = 0,
  ON = 1
};

enum LedMode {
  BLINK = 0,
  ALWAYS_ON = 1,
  ALWAYS_OFF = 2
};
```

`Створити клас-конфігурацію з static const для додаткових налаштувань` \
**SerialConfiguration** зберігає налаштування Serial: частоту монітору і періодичність логу з пункту 3 
``` c
class SerialConfiguration {
public: 
  static const int MONITOR_SPEED = 115200;
  static const int LOOPS_PER_LOG = 1000;
};
```

`Створити клас Led з методами init() та set(LedState state)` \
Класс **Led** інкапсулює взаємодію з світлодіодом. Він містить логіку ініціалізації, встановлення і запам'ятовування значення

``` c
class Led {
  const int pin;
  bool initialized = false;
  LedState currentLedState = LedState::OFF;


public:
  Led(int ledPin) : pin(ledPin) {}

  void init() {  
    if (!initialized) {
      pinMode(pin, OUTPUT);
      initialized = true;
    }
  }

  void set(LedState state) {
    if (initialized) {
      digitalWrite(pin, (uint8_t)state);
      currentLedState = state;
    }
  }

  LedState get() {
    return currentLedState;
  } 

};
```

`Під'єднати кнопку до GPIO, налаштувати переривання (attachInterrupt)` \
`Створити volatile bool buttonPressed для сигналізації про натиск` \
`ISR має бути мінімальною` \
`Не використовувати Serial.print всередині ISR` \
Класс **Button** інкапсулює взаємодію з кнопкою. Він містить логіку ініціалізації, реакції на натискання(IRS), дебаунсу, та запам'ятовує останній момент часу коли кнопка була натсинута. \
Логіка дебаунсу в методі **getLastDebouncedPressedMicros()** не є ідеальною і може призводити до "пропуску" натискань, але для нашого випадку, з високою частотою ітерацій головного циклу, це маловірогідно.  
``` c
class Button {
  static Button* instance;
  const int pin;
  bool initialized = false;
  volatile bool pressed = false;
  volatile unsigned long lastPressed = 0;
  unsigned long lastDebouncedPressed = 0;

public:
  Button(int buttonPin) : pin(buttonPin) {}

  void init() {  
    if (!initialized) {
      Button::instance = this;
      pinMode(pin, INPUT);
      attachInterrupt(digitalPinToInterrupt(pin), handleButtonPress, FALLING);
      initialized = true;
    }
  }

  // I'm aware that using this approach may cause "lost" butoon presses
  // but i think this shouldn't be an issue in this particular application
  // because main loop iterates in around 1 micro
  unsigned long getLastDebouncedPressedMicros() {
    if (pressed && (micros() - lastPressed > DEBOUNCE_MICROS)) {
      if (digitalRead(pin) == LOW) {
        lastDebouncedPressed = lastPressed;
      }
      pressed = false;
    }
    return lastDebouncedPressed;
  }

private:
  static void handleButtonPress() {
    if (Button::instance != nullptr) {
      Button::instance->lastPressed = micros();
      Button::instance->pressed = true;
    }
  }

};
Button* Button::instance = nullptr;
```

Константні вказівники що зберагають об'єкти кнопки і  світлодіода
``` c
static Led *const LED = new Led(LED_PIN);
static Button *const BUTTON = new Button(BUTTON_PIN);
```

Виклик методів що ініціалізуть переферію
``` c
void setup() {
  Serial.begin(SerialConfiguration::MONITOR_SPEED);
  LED->init();
  BUTTON->init();
}
```

`Уникнути глобальних змінних` \
Для уникнення використання глобальних зміних, усі змінні що не використовуються поза циклом, але мають зберігати стан між ітераціями декларуємо в ньому як static
``` c
void loop() {
  static unsigned long blinkStarted = micros();
  static int iterationsCounter = 0;
  static unsigned long iterationDurationMesturementStart = micros();
  
  static unsigned long stateSwitchCounter = 0;
  
  static unsigned long lastProcessedPress = 0;
  static LedMode currentLedMode = BLINK;

  unsigned long iterationStarted = micros();
  iterationsCounter++;
```

`Логіку перемикання режиму обробляти в loop()` \
Зчитуємо актуальне значенн **lastDebouncedPressedMicros**, якщо воно новіше з останє оброблене - перемикаємо режим. Також, якщо режим перемикається на **BLINK**, скидаємо значення змінних важливих для його роботи
``` c
  unsigned long lastDebouncedPressedMicros = BUTTON->getLastDebouncedPressedMicros();
  if(lastProcessedPress < lastDebouncedPressedMicros) {
    currentLedMode = static_cast<LedMode>((currentLedMode + 1) % NUMBER_OF_MODES);
    lastProcessedPress = lastDebouncedPressedMicros;
    Serial.printf("Changed mode to: %d\r\n", currentLedMode); 

    // reset blink started timestamp and counter
    if (currentLedMode == BLINK) {
      blinkStarted = micros();
      stateSwitchCounter = 0;
    }
  }
```
`Коли кнопка натиснута, LED змінює режим: блимання → постійно увімкнено → постійно вимкнено → назад до блимання` \
**switch** перевіряє актульний режим і виконує відповідний до нього код

`Не використовувати delay(); зробити неблокуючий код (superloop). Використати millis() для керування часом` \
Для **BLINK** Розраховуємо час що пройшов з останнього перемикання світлодіода, якщо він більший за період блимання - перемикаємо стан світлодіода
``` c
  switch (currentLedMode){
    case BLINK: 
      // (now - blinkStarted - counter*BLINK_PERIOD_MICROS) calculates time from prevoius state switch
      // if time passed >= BLINK_PERIOD_MICROS -> switch state 
      if (iterationStarted - blinkStarted - stateSwitchCounter*BLINK_PERIOD_MICROS >= BLINK_PERIOD_MICROS) {
        LED->set(LED->get() == LedState::ON ? LedState::OFF : LedState::ON);
        stateSwitchCounter++;
      }
      break;
    case ALWAYS_ON: 
      if (LED->get() == LedState::OFF) {
        LED->set(LedState::ON);
      }
      break;
    case ALWAYS_OFF:
      if (LED->get() == LedState::ON) {
        LED->set(LedState::OFF);
      }
      break;
  }
```
`Виміряти час однієї ітерації loop() без delay()` \
`Вивести у Serial Monitor кожні 1000 ітерацій` \
Середній час на ітерацю вираховуємо як різницю між **iterationDurationMesturementStart** що оновлюється раз на **SerialConfiguration::LOOPS_PER_LOG** ітерацій та **micros**() поділену на кількість ітерацій маж замірами.
Час виконання ітерації неймовірно швидкий, тож виміряти його фбсолютно точно не є можливим через обмеження гранулярності та точності micros()
``` c
  if (iterationsCounter % SerialConfiguration::LOOPS_PER_LOG == 0) {
    Serial.printf("Time to iterate micros: %f\r\n", 
      (micros()-iterationDurationMesturementStart)/(float)SerialConfiguration::LOOPS_PER_LOG, iterationsCounter); 
    iterationsCounter = 0; // reset counter to avoid overflow
    iterationDurationMesturementStart = micros();
  }
} 
```
Приклад логу
```
Time to iterate micros: 1.384000
Time to iterate micros: 1.390000
Time to iterate micros: 1.383000
Time to iterate micros: 1.383000
Time to iterate micros: 1.384000
Time to iterate micros: 1.391000
Time to iterate micros: 1.383000
Time to iterate micros: 1.383000
Time to iterate micros: 1.390000
Time to iterate micros: 1.384000
Time to iterate micros: 1.390000
Time to iterate micros: 1.384000
Time to iterate micros: 1.389000
Time to iterate micros: 1.384000
Time to iterate micros: 1.390000
```