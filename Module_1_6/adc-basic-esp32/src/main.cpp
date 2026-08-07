#include <stdlib.h>
#include <Arduino.h>

const int adcPin = 4; // GPIO 4 (ADC1 Channel 3)

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_6db); // Set max range to ~3.3V
  // analogReadResolution(10);
  Serial.printf("|  Raw | Caclculated Voltage | Measured Voltage |  Error |\n");
}

void loop() {
  uint16_t rawValue = analogRead(adcPin);
  float measuredVoltage = (float)analogReadMilliVolts(adcPin)/1000;
  // float calculatedVoltage = rawValue * (3.3 / 1023.0); // 12-bit resolution
  float calculatedVoltage = rawValue * (3.3 / 4095.0); // 12-bit resolution
  float absoluteError = abs(calculatedVoltage - measuredVoltage) / measuredVoltage;
  
  Serial.printf("| %4d | %19.2f | %16.2f | %5.1f%% |\n", rawValue, calculatedVoltage, measuredVoltage, absoluteError*100);
  delay(1000);
}