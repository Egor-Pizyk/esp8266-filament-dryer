#include <Arduino.h>
#include "sensors/sensors.h"
#include "sensors/aht20/aht20.h"
#include "sensors/thermistor/thermistor.h"
#include "actuators/fan/fan.h"
#include "actuators/dryer_heater/dryer_heater.h"
#include "wifi/wifi.h"
#include "web/web_server.h"

void setup() {
    Serial.begin(115200);
    initSensors();
    initFan();
    initDryerHeater();
    initWifi();
    initWebServer();
}

#define UPDATE_INTERVAL_MS 1000

void loop() {
    handleWebServer();

    static unsigned long lastUpdate = 0;
    unsigned long now = millis();
    if (now - lastUpdate < UPDATE_INTERVAL_MS) {
        return;
    }
    lastUpdate = now;

    float temperature;
    float humidity;
    readAht20(temperature, humidity);
    Serial.printf("AHT20: temp=%.2f C, humidity=%.2f %%\n", temperature, humidity);
    float thermistorTemperature = readThermistor();
    updateFan(thermistorTemperature);

    if (temperature < 45.0) {
        startDryerHeater(thermistorTemperature);
    } else {
        stopDryerHeater();
    }

    setStatus(temperature, humidity, thermistorTemperature, isFanOn(), isDryerHeaterOn());
}
