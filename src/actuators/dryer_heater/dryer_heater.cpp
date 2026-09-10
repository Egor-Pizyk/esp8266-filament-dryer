#include "dryer_heater.h"
#include <Arduino.h>

#define DRYER_HEATER_PIN 5
#define STOP_TEMPERATURE_THRESHOLD 60.0

static bool dryerHeaterOn = false;

void initDryerHeater() {
    pinMode(DRYER_HEATER_PIN, OUTPUT);
    digitalWrite(DRYER_HEATER_PIN, LOW);
}

void startDryerHeater(float temperature) {
    dryerHeaterOn = temperature < STOP_TEMPERATURE_THRESHOLD;
    digitalWrite(DRYER_HEATER_PIN, dryerHeaterOn ? HIGH : LOW);
}

void stopDryerHeater() {
    dryerHeaterOn = false;
    digitalWrite(DRYER_HEATER_PIN, LOW);
}

bool isDryerHeaterOn() {
    return dryerHeaterOn;
}
