// =============================================================
// BasicTelemetry_mqtt — Autoconnecto SDK example
//
// Pair sketch: examples/BasicTelemetry_http/BasicTelemetry_http.ino
//   (HTTPS device-token API — no `AutoconnectoSDK` calls; install library for TLS header.)
//
// PURPOSE
//   Simplest possible sketch. Connect to the platform and send
//   sensor readings on a plan-safe interval. No attributes, no RPC.
//   Use this first to verify your device token and connection.
//
// PLAN INTERVAL
//   Free plan stores at most one reading every 30 seconds. Hobby 10s,
//   Starter 5s, Growth/Enterprise 1s. This sketch defaults to 35s and
//   sends ALL keys in ONE MQTT message so none are dropped.
//
// NOTE
//   Raspberry Pi mirrors of these examples live under `raspberrypi/`.
//
// DASHBOARD WIDGETS THAT WORK WITH THIS SKETCH
//   - Any chart/telemetry widget configured with:
//     temperature, humidity, current, power,
//     voltage1, voltage2, voltage3
//
// KEYS USED (same across all examples — one dashboard for all)
//   Telemetry : temperature, humidity, current, power,
//               voltage1, voltage2, voltage3
//   Client    : freeHeap, wifiRSSI, uptime, sdkVersion
// =============================================================

#include <AutoconnectoSDK.h>
// Copy secrets.h.example → secrets.h (gitignored) and fill Wi‑Fi + device token.
#include "secrets.h"

AutoconnectoSDK sdk;

// TLS trust store: AUTOCONNECTO_ROOT_CA from AutoconnectoIsrgRoots.h (pulled in by AutoconnectoSDK.h)

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(2000);

  SDKConfig config;

  // ---- WiFi ----
  config.wifiSSID     = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;

  // ---- MQTT / WSS ----
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort  = 8084;

  // ---- Device ----
  config.deviceToken = DEVICE_TOKEN;

  // ---- Transport ----
  config.enableWS   = true;
  config.enableMQTT = true;

  // ---- TLS ----
  config.allowInsecureTLS = false;
  config.rootCA           = AUTOCONNECTO_ROOT_CA;

  // ---- Debug ----
  config.enableSerialLogs = true;

  sdk.begin(config);

  Serial.println("[SDK] BasicTelemetry_mqtt started");
}

// ============================================================
// LOOP
// ============================================================

unsigned long lastTelemetry   = 0;
unsigned long lastAttrReport  = 0;

void loop() {

  // CRITICAL: must run every iteration — services MQTT keepalive.
  // Never delay() more than ~50ms here.
  sdk.loop();

  unsigned long now = millis();

  // ---- One MQTT telemetry message every 35s (Free-plan safe) ----
  if (now - lastTelemetry > 35000) {
    lastTelemetry = now;

    StaticJsonDocument<256> reading;
    reading["temperature"] = (float)random(20, 35);
    reading["humidity"] = (float)random(40, 80);
    reading["current"] = (float)random(1, 10);
    reading["power"] = (float)random(100, 500);
    reading["voltage1"] = (float)random(220, 280);
    reading["voltage2"] = (float)random(220, 280);
    reading["voltage3"] = (float)random(220, 280);
    sdk.sendTelemetry(reading);

    Serial.println("[TEL] Sent telemetry (one message)");
  }

  // ---- Report device health as client attributes every 30 seconds ----
  if (now - lastAttrReport > 30000) {
    lastAttrReport = now;

    StaticJsonDocument<128> attrs;
    attrs["freeHeap"]   = ESP.getFreeHeap();
    attrs["wifiRSSI"]   = WiFi.RSSI();
    attrs["uptime"]     = millis() / 1000;
    attrs["sdkVersion"] = 1.0;
    sdk.sendClientAttributes(attrs);

    Serial.println("[ATTR] Sent device health");
  }

  delay(10);
}
