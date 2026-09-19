#include "web_server.h"
#include "../sensors/thermistor/thermistor.h"
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
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Dryer dashboard</title>
<style>
:root {
  --bg: #0b0d10;
  --surface: #14171c;
  --border: #23272e;
  --text: #e6e8eb;
  --muted: #8b929c;
  --accent: #4c8dff;
  --good: #34c77b;
  --idle: #4a5058;
  --radius: 14px;
}
* { box-sizing: border-box; }
body {
  font-family: -apple-system, "Segoe UI", Roboto, sans-serif;
  background: radial-gradient(circle at top, #10131a 0%, var(--bg) 60%);
  color: var(--text);
  margin: 0;
  padding: 1.5rem;
}
h1 { font-size: 1.1rem; font-weight: 600; color: var(--muted); letter-spacing: .04em; text-transform: uppercase; margin: 0 0 1rem; }
.grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 1rem;
  margin-bottom: 1.5rem;
}
.card {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 1rem 1.2rem;
}
.card .label { color: var(--muted); font-size: .8rem; }
.card .value { font-size: 1.9rem; font-weight: 700; margin-top: .25rem; }
.badge {
  display: inline-flex; align-items: center; gap: .4rem;
  font-size: .95rem; font-weight: 600; margin-top: .25rem;
  color: var(--idle);
}
.badge::before { content: ""; width: 8px; height: 8px; border-radius: 50%; background: var(--idle); }
.badge.on { color: var(--good); }
.badge.on::before { background: var(--good); box-shadow: 0 0 8px var(--good); }
.chart-card { background: var(--surface); border: 1px solid var(--border); border-radius: var(--radius); padding: 1rem 1.2rem; }
.chart-card .label { color: var(--muted); font-size: .8rem; display: flex; justify-content: space-between; margin-bottom: .5rem; }
canvas { width: 100%; height: 160px; display: block; }
.legend { display: flex; gap: 1rem; margin-top: .6rem; flex-wrap: wrap; }
.legend .item { display: flex; align-items: center; gap: .4rem; font-size: .82rem; color: var(--muted); }
.legend .swatch { width: 10px; height: 10px; border-radius: 50%; flex: none; }
.legend .item .value { color: var(--text); font-weight: 600; font-size: .82rem; margin: 0; }
</style>
</head>
<body>
<h1>Dryer dashboard</h1>
<div class="grid">
  <div class="card">
    <div class="label">Chamber temperature (AHT20)</div>
    <div class="value" id="chamberTemperature">--</div>
  </div>
  <div class="card">
    <div class="label">Chamber humidity (AHT20)</div>
    <div class="value" id="chamberHumidity">--</div>
  </div>
  <div class="card">
    <div class="label">Heater temperature (thermistor)</div>
    <div class="value" id="heaterTemperature">--</div>
  </div>
  <div class="card">
    <div class="label">Fan</div>
    <div class="badge" id="fanOn">--</div>
  </div>
  <div class="card">
    <div class="label">Heater</div>
    <div class="badge" id="heaterOn">--</div>
  </div>
</div>
<div class="grid">
  <div class="chart-card">
    <div class="label"><span>Temperature</span><span>last 10 min</span></div>
    <canvas id="tempChart"></canvas>
    <div class="legend" id="tempLegend"></div>
  </div>
  <div class="chart-card">
    <div class="label"><span>Humidity</span><span>last 10 min</span></div>
    <canvas id="humidityChart"></canvas>
    <div class="legend" id="humidityLegend"></div>
  </div>
</div>

<!-- ponytail: temporary raw thermistor debug panel, delete once resistance issue is diagnosed -->
<div style="border:2px dashed orange; padding:1rem; font-family:monospace; font-size:.85rem; margin-bottom:1.5rem;">
  <b>THERMISTOR DEBUG (temporary)</b>
  <div id="debugNow"></div>
  <div id="debugHistory" style="max-height:200px; overflow-y:auto; margin-top:.5rem; white-space:pre;"></div>
</div>

<script>
const HISTORY_MS = 10 * 60 * 1000;
const history = []; // {t, chamberTemperature, heaterTemperature, chamberHumidity}

function pruneHistory(now) {
    while (history.length && now - history[0].t > HISTORY_MS) history.shift();
}

const SERIES_LABEL = { chamberTemperature: "Chamber", heaterTemperature: "Heater", chamberHumidity: "Humidity" };
const SERIES_UNIT = { chamberTemperature: "°C", heaterTemperature: "°C", chamberHumidity: "%" };
const SERIES_COLOR = { chamberTemperature: "#4c8dff", heaterTemperature: "#ff8a4c", chamberHumidity: "#34c77b" };

function drawChart(canvas, series) {
    const ctx = canvas.getContext("2d");
    const dpr = window.devicePixelRatio || 1;
    const w = canvas.clientWidth, h = canvas.clientHeight;
    canvas.width = w * dpr;
    canvas.height = h * dpr;
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, w, h);
    if (history.length < 2) return;

    let min = Infinity, max = -Infinity;
    for (const p of history) {
        for (const key of series) {
            if (p[key] < min) min = p[key];
            if (p[key] > max) max = p[key];
        }
    }
    if (min === max) { min -= 1; max += 1; }
    const pad = (max - min) * 0.1;
    min -= pad; max += pad;

    const t0 = history[0].t, t1 = history[history.length - 1].t;
    const span = Math.max(t1 - t0, 1);

    ctx.font = "11px -apple-system, sans-serif";
    ctx.fillStyle = "#8b929c";
    ctx.textAlign = "right";
    ctx.textBaseline = "top";
    ctx.fillText(max.toFixed(1), w - 2, 2);
    ctx.textBaseline = "bottom";
    ctx.fillText(min.toFixed(1), w - 2, h - 14);

    const fmtTime = ms => new Date(ms).toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" });
    ctx.textBaseline = "bottom";
    ctx.textAlign = "left";
    ctx.fillText(fmtTime(t0), 2, h);
    ctx.textAlign = "right";
    ctx.fillText(fmtTime(t1), w - 2, h);
    ctx.textAlign = "left";

    for (const key of series) {
        ctx.beginPath();
        ctx.strokeStyle = SERIES_COLOR[key];
        ctx.lineWidth = 2;
        let lastX, lastY;
        history.forEach((p, i) => {
            const x = ((p.t - t0) / span) * w;
            const y = h - ((p[key] - min) / (max - min)) * h;
            if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
            lastX = x; lastY = y;
        });
        ctx.stroke();

        ctx.beginPath();
        ctx.fillStyle = SERIES_COLOR[key];
        ctx.arc(lastX, lastY, 3.5, 0, Math.PI * 2);
        ctx.fill();
    }
}

function renderLegend(el, series) {
    const latest = history[history.length - 1];
    el.innerHTML = series.map(key => `
        <span class="item">
            <span class="swatch" style="background:${SERIES_COLOR[key]}"></span>
            ${SERIES_LABEL[key]}
            <span class="value">${latest ? latest[key].toFixed(1) : "--"} ${SERIES_UNIT[key]}</span>
        </span>
    `).join("");
}

async function refresh() {
    try {
        const res = await fetch("/status");
        const data = await res.json();
        const now = Date.now();

        document.getElementById("chamberTemperature").textContent = data.chamberTemperature.toFixed(1) + " °C";
        document.getElementById("chamberHumidity").textContent = data.chamberHumidity.toFixed(1) + " %";
        document.getElementById("heaterTemperature").textContent = data.heaterTemperature.toFixed(1) + " °C";

        const fanEl = document.getElementById("fanOn");
        fanEl.textContent = data.fanOn ? "Spinning" : "Off";
        fanEl.className = "badge " + (data.fanOn ? "on" : "");

        const heaterEl = document.getElementById("heaterOn");
        heaterEl.textContent = data.heaterOn ? "Heating" : "Off";
        heaterEl.className = "badge " + (data.heaterOn ? "on" : "");

        history.push({
            t: now,
            chamberTemperature: data.chamberTemperature,
            heaterTemperature: data.heaterTemperature,
            chamberHumidity: data.chamberHumidity
        });
        pruneHistory(now);

        drawChart(document.getElementById("tempChart"), ["chamberTemperature", "heaterTemperature"]);
        drawChart(document.getElementById("humidityChart"), ["chamberHumidity"]);
        renderLegend(document.getElementById("tempLegend"), ["chamberTemperature", "heaterTemperature"]);
        renderLegend(document.getElementById("humidityLegend"), ["chamberHumidity"]);

        // ponytail: temporary debug panel, delete with the panel above
        debugHistory.push({
            t: now,
            adc: data.thermistorAdc,
            voltage: data.thermistorVoltage,
            resistance: data.thermistorResistance,
            temp: data.heaterTemperature
        });
        while (debugHistory.length && now - debugHistory[0].t > HISTORY_MS) debugHistory.shift();

        document.getElementById("debugNow").textContent =
            `adc=${data.thermistorAdc}  voltage=${data.thermistorVoltage.toFixed(3)}V  resistance=${data.thermistorResistance.toFixed(1)}ohm  temp=${data.heaterTemperature.toFixed(1)}C`;
        document.getElementById("debugHistory").textContent = debugHistory.slice().reverse().map(p =>
            `${new Date(p.t).toLocaleTimeString()}  adc=${p.adc}  V=${p.voltage.toFixed(3)}  R=${p.resistance.toFixed(1)}  T=${p.temp.toFixed(1)}`
        ).join("\n");
    } catch (e) {}
}
const debugHistory = [];
refresh();
setInterval(refresh, 2000);
window.addEventListener("resize", refresh);
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
    json += "\"heaterOn\":" + String(statusHeaterOn ? "true" : "false") + ",";
    json += "\"thermistorAdc\":" + String(getLastThermistorAdc()) + ",";
    json += "\"thermistorVoltage\":" + String(getLastThermistorVoltage(), 3) + ",";
    json += "\"thermistorResistance\":" + String(getLastThermistorResistance(), 1);
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
