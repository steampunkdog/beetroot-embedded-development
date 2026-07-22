#include <Arduino.h>


#define RED_LED_OUT 15  // Пін до якого підключено червоний світлодіод
#define BLUE_LED_OUT 16 // Пін до якого підключено синій світлодіод

void setup() {
    // Налаштовуємо піни як вихіди (OUTPUT), щоб контролер міг подавати на них напругу
    pinMode(RED_LED_OUT, OUTPUT);
    pinMode(BLUE_LED_OUT, OUTPUT);
}

void loop() {
    digitalWrite(RED_LED_OUT, HIGH);  // Вмикаємо червоний світлодіод
    delay(400);                       // Чекаємо 100 мілісекунд

    digitalWrite(BLUE_LED_OUT, LOW);  // Вимикаємо синій світлодіод
    digitalWrite(RED_LED_OUT, LOW);   // Вимикаємо червоний світлодіод
    delay(200);                       // Чекаємо 100 мілісекунд

    digitalWrite(RED_LED_OUT, HIGH);   // Вимикаємо червоний світлодіод
    delay(400);                       // Чекаємо 100 мілісекунд

    digitalWrite(BLUE_LED_OUT, HIGH); // Вмикаємо синій світлодіод
    delay(400);                       // Чекаємо 200 мілісекунд
    
    digitalWrite(RED_LED_OUT, LOW);   // Вимикаємо червоний світлодіод
    delay(400);                       // Чекаємо 200 мілісекунд

    digitalWrite(BLUE_LED_OUT, LOW);  // Вимикаємо синій світлодіод
    delay(200);                       // Чекаємо 100 мілісекунд

    digitalWrite(BLUE_LED_OUT, HIGH); // Вмикаємо синій світлодіод
    delay(400);                       // Чекаємо 200 мілісекунд
}