#include "sensors.h"
#include <Arduino.h>
#include <Wire.h>
#include "aht20/aht20_init.h"
#include "thermistor/thermistor_init.h"

#define I2C_SDA_PIN 13
#define I2C_SCL_PIN 12

void initSensors() {
    // I2C: SDA = D2 (GPIO4), SCL = D1 (GPIO5)
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    Serial.println("Starting sensors...");

    initAht20();
    initThermistor();
}
