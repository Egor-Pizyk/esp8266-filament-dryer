#include "aht20_init.h"
#include "aht20.h"
#include <Arduino.h>

void initAht20() {
    if (!aht.begin()) {
        Serial.println("AHT20 not found!");
    } else {
        Serial.println("AHT20 OK");
    }
}
