#include <Arduino.h>


#define RED_LED_OUT 15  // Пін до якого підключено червоний світлодіод
#define BLUE_LED_OUT 16 // Пін до якого підключено синій світлодіод
#define GREEN_LED_OUT 17 // Пін до якого підключено синій світлодіод

const int pins[3] = {RED_LED_OUT, BLUE_LED_OUT, GREEN_LED_OUT};

void setup() {
    // Налаштовуємо піни як вихіди (OUTPUT), щоб контролер міг подавати на них напругу
    pinMode(RED_LED_OUT, OUTPUT);
    pinMode(BLUE_LED_OUT, OUTPUT);
    pinMode(GREEN_LED_OUT, OUTPUT);
}

void loop() {
  static int i = 0; // Декларуємо статичну змінну що вказуватиме на поточний індекс в масиві
  
  digitalWrite(pins[i], LOW);  // Вимикаємо попередній світлодіод
  
  i = (i + 1) % 3;             // Обчислюємо індекс наступного діода
  digitalWrite(pins[i], HIGH); // Вмикаємо наступний світлодіод

  delay(400);
}