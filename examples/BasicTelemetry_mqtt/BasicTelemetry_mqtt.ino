// =============================================================
// BasicTelemetry_mqtt — Autoconnecto SDK example
//
// Pair sketch: examples/BasicTelemetry_http/BasicTelemetry_http.ino
//   (HTTPS device-token API — no `AutoconnectoSDK` calls; install library for TLS header.)
//
// PURPOSE
//   Simplest possible sketch. Connect to the platform and send
//   sensor readings every 10 seconds. No attributes, no RPC.
//   Use this first to verify your device token and connection.
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
  config.wifiSSID     = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";

  // ---- MQTT / WSS ----
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort  = 8084;

  // ---- Device ----
  config.deviceToken = "YOUR_DEVICE_TOKEN";

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

  // ---- Send telemetry every 10 seconds ----
  if (now - lastTelemetry > 10000) {
    lastTelemetry = now;

    // Single-key calls (up to 3 keys per variadic call)
    sdk.sendTelemetry("temperature", (float)random(20, 35));
    sdk.sendTelemetry(
      "humidity", (float)random(40, 80),
      "current",  (float)random(1, 10),
      "power",    (float)random(100, 500)
    );

    // 4+ keys require a JsonDocument
    StaticJsonDocument<128> voltages;
    voltages["voltage1"] = (float)random(220, 280);
    voltages["voltage2"] = (float)random(220, 280);
    voltages["voltage3"] = (float)random(220, 280);
    sdk.sendTelemetry(voltages);

    Serial.println("[TEL] Sent telemetry");
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
