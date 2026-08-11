#include <Arduino.h>

constexpr int LED_PIN = 15;
constexpr int BUTTON_PIN = 16;
constexpr int BLINK_PERIOD_MICROS = 300000; // 300ms
constexpr int DEBOUNCE_MICROS = 5000;       // 5ms
constexpr int NUMBER_OF_MODES = 3;

enum class LedState {
  OFF = 0,
  ON = 1
};

enum LedMode {
  BLINK = 0,
  ALWAYS_ON = 1,
  ALWAYS_OFF = 2
};

class SerialConfiguration {
public: 
  static const int MONITOR_SPEED = 115200;
  static const int LOOPS_PER_LOG = 1000;
};

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

static Led *const LED = new Led(LED_PIN);
static Button *const BUTTON = new Button(BUTTON_PIN);

void setup() {
  Serial.begin(SerialConfiguration::MONITOR_SPEED);
  LED->init();
  BUTTON->init();
}

void loop() {
  static unsigned long blinkStarted = micros();
  static int iterationsCounter = 0;
  static unsigned long iterationDurationMesturementStart = micros();
  
  static unsigned long stateSwitchCounter = 0;
  
  static unsigned long lastProcessedPress = 0;
  static LedMode currentLedMode = BLINK;

  unsigned long iterationStarted = micros();
  iterationsCounter++;

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

  if (iterationsCounter % SerialConfiguration::LOOPS_PER_LOG == 0) {
    Serial.printf("Time to iterate micros: %f\r\n", 
      (micros()-iterationDurationMesturementStart)/(float)SerialConfiguration::LOOPS_PER_LOG, iterationsCounter); 
    iterationsCounter = 0; // reset counter to avoid overflow
    iterationDurationMesturementStart = micros();
  }
} 
