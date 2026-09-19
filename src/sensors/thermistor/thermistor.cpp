#include "thermistor.h"
#include <Arduino.h>
#include <math.h>

#define THERMISTOR_PIN A0
#define SERIES_RESISTOR 10000.0
#define NOMINAL_RESISTANCE 10000.0
#define NOMINAL_TEMPERATURE 25.0
#define B_COEFFICIENT 3977.0
#define ADC_MAX 1023.0
#define VCC 3.3

static int lastAdc = 0;
static float lastVoltage = 0;
static float lastResistance = 0;

int getLastThermistorAdc() { return lastAdc; }
float getLastThermistorVoltage() { return lastVoltage; }
float getLastThermistorResistance() { return lastResistance; }

float readThermistor() {
    int adc = analogRead(THERMISTOR_PIN);
    float voltage = (adc / ADC_MAX) * VCC;
    float resistance = SERIES_RESISTOR * voltage / (VCC - voltage);

    // Beta equation
    float steinhart = resistance / NOMINAL_RESISTANCE;
    steinhart = log(steinhart);
    steinhart /= B_COEFFICIENT;
    steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15);
    steinhart = 1.0 / steinhart;
    steinhart -= 273.15;

    lastAdc = adc;
    lastVoltage = voltage;
    lastResistance = resistance;

    // ponytail: temp debug logging, remove once resistance mismatch is diagnosed
    Serial.print("[thermistor] adc=");
    Serial.print(adc);
    Serial.print(" voltage=");
    Serial.print(voltage, 3);
    Serial.print("V resistance=");
    Serial.print(resistance, 1);
    Serial.print("ohm temp=");
    Serial.println(steinhart, 1);

    return steinhart;
}
