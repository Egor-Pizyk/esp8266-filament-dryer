#include "fan.h"
#include <Arduino.h>

#define FAN_PIN 16
#define TEMPERATURE_THRESHOLD 25.0

static bool fanOn = false;

void initFan() {
    pinMode(FAN_PIN, OUTPUT);
    digitalWrite(FAN_PIN, LOW);
}

void updateFan(float temperature) {
    fanOn = temperature >= TEMPERATURE_THRESHOLD;
    digitalWrite(FAN_PIN, fanOn ? HIGH : LOW);
}

bool isFanOn() {
    return fanOn;
}
