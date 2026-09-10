#include "thermistor_init.h"
#include <Arduino.h>

#define THERMISTOR_PIN A0

void initThermistor() {
    pinMode(THERMISTOR_PIN, INPUT);
    Serial.println("Thermistor OK");
}
