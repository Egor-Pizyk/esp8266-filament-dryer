#include "wifi.h"
#include "wifi_credentials.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>

void initWifi() {
    Serial.print("Connecting to WiFi \"");
    Serial.print(WIFI_SSID);
    Serial.println("\"...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
}
