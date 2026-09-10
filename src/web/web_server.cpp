#include "web_server.h"
#include <Arduino.h>
#include <ESP8266WebServer.h>

static ESP8266WebServer server(80);

static float statusChamberTemperature = 0;
static float statusChamberHumidity = 0;
static float statusHeaterTemperature = 0;
static bool statusFanOn = false;
static bool statusHeaterOn = false;

static const char PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>Dryer status</title>
<style>
body { font-family: sans-serif; background: #111; color: #eee; margin: 0; padding: 2rem; }
h1 { font-size: 1.2rem; color: #888; }
.row { display: flex; justify-content: space-between; padding: 0.75rem 0; border-bottom: 1px solid #333; }
.value { font-weight: bold; }
.on { color: #4caf50; }
.off { color: #777; }
</style>
</head>
<body>
<h1>Dryer status</h1>
<div class="row"><span>Chamber temperature (AHT20)</span><span class="value" id="chamberTemperature">--</span></div>
<div class="row"><span>Chamber humidity (AHT20)</span><span class="value" id="chamberHumidity">--</span></div>
<div class="row"><span>Heater temperature (thermistor)</span><span class="value" id="heaterTemperature">--</span></div>
<div class="row"><span>Fan</span><span class="value" id="fanOn">--</span></div>
<div class="row"><span>Heater</span><span class="value" id="heaterOn">--</span></div>
<script>
async function refresh() {
    try {
        const res = await fetch("/status");
        const data = await res.json();
        document.getElementById("chamberTemperature").textContent = data.chamberTemperature.toFixed(1) + " °C";
        document.getElementById("chamberHumidity").textContent = data.chamberHumidity.toFixed(1) + " %";
        document.getElementById("heaterTemperature").textContent = data.heaterTemperature.toFixed(1) + " °C";
        const fanEl = document.getElementById("fanOn");
        fanEl.textContent = data.fanOn ? "spinning" : "off";
        fanEl.className = "value " + (data.fanOn ? "on" : "off");
        const heaterEl = document.getElementById("heaterOn");
        heaterEl.textContent = data.heaterOn ? "on" : "off";
        heaterEl.className = "value " + (data.heaterOn ? "on" : "off");
    } catch (e) {}
}
refresh();
setInterval(refresh, 2000);
</script>
</body>
</html>
)HTML";

static void handleRoot() {
    server.send_P(200, "text/html", PAGE);
}

static void handleStatus() {
    String json = "{";
    json += "\"chamberTemperature\":" + String(statusChamberTemperature, 1) + ",";
    json += "\"chamberHumidity\":" + String(statusChamberHumidity, 1) + ",";
    json += "\"heaterTemperature\":" + String(statusHeaterTemperature, 1) + ",";
    json += "\"fanOn\":" + String(statusFanOn ? "true" : "false") + ",";
    json += "\"heaterOn\":" + String(statusHeaterOn ? "true" : "false");
    json += "}";
    server.send(200, "application/json", json);
}

void initWebServer() {
    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.begin();
    Serial.println("Web server started");
}

void handleWebServer() {
    server.handleClient();
}

void setStatus(float chamberTemperature, float chamberHumidity, float heaterTemperature, bool fanOn, bool heaterOn) {
    statusChamberTemperature = chamberTemperature;
    statusChamberHumidity = chamberHumidity;
    statusHeaterTemperature = heaterTemperature;
    statusFanOn = fanOn;
    statusHeaterOn = heaterOn;
}
