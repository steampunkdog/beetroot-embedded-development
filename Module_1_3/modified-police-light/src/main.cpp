#include <Arduino.h>


#define RED_LED_OUT 15  // Пін до якого підключено червоний світлодіод
#define BLUE_LED_OUT 16 // Пін до якого підключено синій світлодіод

void setup() {
    // Налаштовуємо піни як вихіди (OUTPUT), щоб контролер міг подавати на них напругу
    pinMode(RED_LED_OUT, OUTPUT);
    pinMode(BLUE_LED_OUT, OUTPUT);
}

void loop() {
    // Обидва світлодіоди увімкнено
    digitalWrite(BLUE_LED_OUT, HIGH); 
    digitalWrite(RED_LED_OUT, HIGH);  
    delay(200);

    // Тільки червоний світлодіод увімкнено
    digitalWrite(BLUE_LED_OUT, LOW);
    digitalWrite(RED_LED_OUT, HIGH);
    delay(400);

    // Обидва світлодіоди вимкнено
    digitalWrite(BLUE_LED_OUT, LOW);
    digitalWrite(RED_LED_OUT, LOW);
    delay(200);
    
    // Тільки червоний світлодіод увімкнено
    digitalWrite(BLUE_LED_OUT, LOW);
    digitalWrite(RED_LED_OUT, HIGH);
    delay(400);

    // Обидва світлодіоди увімкнено
    digitalWrite(BLUE_LED_OUT, HIGH);
    digitalWrite(RED_LED_OUT, HIGH);
    delay(200);

    // Тільки синій світлодіод увімкнено
    digitalWrite(BLUE_LED_OUT, HIGH);
    digitalWrite(RED_LED_OUT, LOW);
    delay(400);
    
    // Обидва світлодіоди вимкнено
    digitalWrite(BLUE_LED_OUT, LOW);
    digitalWrite(RED_LED_OUT, LOW);
    delay(200);

    // Тільки синій світлодіод увімкнено
    digitalWrite(BLUE_LED_OUT, HIGH);
    digitalWrite(RED_LED_OUT, LOW);
    delay(400);
}