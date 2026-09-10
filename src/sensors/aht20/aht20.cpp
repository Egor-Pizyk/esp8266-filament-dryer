#include "aht20.h"

Adafruit_AHTX0 aht;

void readAht20(float &temperature, float &humidity) {
    sensors_event_t humidityEvent;
    sensors_event_t temperatureEvent;

    aht.getEvent(&humidityEvent, &temperatureEvent);

    temperature = temperatureEvent.temperature;
    humidity = humidityEvent.relative_humidity;
}
