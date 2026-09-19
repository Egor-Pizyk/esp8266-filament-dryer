#include "dryer_heater.h"
#include <Arduino.h>
#include <math.h>

#define DRYER_HEATER_PIN 5
#define STOP_TEMPERATURE_THRESHOLD 60.0
#define HYSTERESIS 10.0
#define START_TEMPERATURE_THRESHOLD (STOP_TEMPERATURE_THRESHOLD - HYSTERESIS)
#define MIN_VALID_TEMPERATURE -40.0
#define MAX_VALID_TEMPERATURE 150.0

static bool dryerHeaterOn = false;

void initDryerHeater() {
    pinMode(DRYER_HEATER_PIN, OUTPUT);
    digitalWrite(DRYER_HEATER_PIN, LOW);
}

void startDryerHeater(float temperature) {
    if (isnan(temperature) || temperature < MIN_VALID_TEMPERATURE || temperature > MAX_VALID_TEMPERATURE) {
        stopDryerHeater();
        return;
    }

    if (dryerHeaterOn) {
        dryerHeaterOn = temperature < STOP_TEMPERATURE_THRESHOLD;
    } else {
        dryerHeaterOn = temperature < START_TEMPERATURE_THRESHOLD;
    }
    digitalWrite(DRYER_HEATER_PIN, dryerHeaterOn ? HIGH : LOW);
}

void stopDryerHeater() {
    dryerHeaterOn = false;
    digitalWrite(DRYER_HEATER_PIN, LOW);
}

bool isDryerHeaterOn() {
    return dryerHeaterOn;
}
