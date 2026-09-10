#pragma once

#include <Adafruit_AHTX0.h>

extern Adafruit_AHTX0 aht;

void readAht20(float &temperature, float &humidity);
