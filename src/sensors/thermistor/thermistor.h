#pragma once

// NTCLE100E3103JB0 divider: 3.3V -> 10k resistor -> A0 node -> thermistor -> GND
float readThermistor();

// ponytail: debug getters for the live-diagnostics UI panel, remove with the panel
int getLastThermistorAdc();
float getLastThermistorVoltage();
float getLastThermistorResistance();
