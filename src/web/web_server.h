#pragma once

void initWebServer();
void handleWebServer();
void setStatus(float chamberTemperature, float chamberHumidity, float heaterTemperature, bool fanOn, bool heaterOn);
