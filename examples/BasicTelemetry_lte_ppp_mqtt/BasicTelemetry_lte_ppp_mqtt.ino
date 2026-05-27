// =============================================================
// BasicTelemetry_lte_ppp_mqtt — Autoconnecto SDK example (LTE PPP / EC200)
//
// PURPOSE
//   LTE-first minimal sketch to validate EC200 PPP bring-up + MQTTS publish.
//   Same telemetry keys as BasicTelemetry_mqtt.
//
// REQUIRED
//   - ESP32 Arduino core 3.x (built-in PPP library)
//   - build_opt.h in this folder: -DAUTOCONNECTO_ENABLE_LTE_PPP=1
//
// NOTES
//   - LTE uses MQTTS :8883 (no WSS).
//   - GPIO 16/17 are default UART2 pins; ensure they're free on your board.
// =============================================================

#include <AutoconnectoSDK.h>

AutoconnectoSDK sdk;

void setup() {
  Serial.begin(115200);
  delay(2000);

  SDKConfig config;

  // ---- LTE PPP (EC200) ----
  config.networkMode = NetworkMode::LtePpp;
  config.lteApn      = "YOUR_CARRIER_APN";
  config.lteUartRx   = 16;
  config.lteUartTx   = 17;
  config.lteResetPin = -1; // set if RESET is wired

  // ---- MQTT ----
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;

  // ---- Device ----
  config.deviceToken = "YOUR_DEVICE_TOKEN";

  // ---- Transport ----
  config.enableWS   = false; // LTE uses MQTTS
  config.enableMQTT = true;

  // ---- TLS ----
  config.allowInsecureTLS = false;
  config.rootCA           = AUTOCONNECTO_ROOT_CA;

  config.enableSerialLogs = true;

  sdk.begin(config);

  Serial.println("[SDK] BasicTelemetry_lte_ppp_mqtt started");
}

unsigned long lastTelemetry   = 0;
unsigned long lastAttrReport  = 0;

void loop() {
  sdk.loop();

  unsigned long now = millis();

  if (now - lastTelemetry > 10000) {
    lastTelemetry = now;

    sdk.sendTelemetry("temperature", (float)random(20, 35));
    sdk.sendTelemetry(
      "humidity", (float)random(40, 80),
      "current",  (float)random(1, 10),
      "power",    (float)random(100, 500)
    );

    StaticJsonDocument<128> voltages;
    voltages["voltage1"] = (float)random(220, 280);
    voltages["voltage2"] = (float)random(220, 280);
    voltages["voltage3"] = (float)random(220, 280);
    sdk.sendTelemetry(voltages);
  }

  if (now - lastAttrReport > 30000) {
    lastAttrReport = now;

    StaticJsonDocument<160> attrs;
    attrs["freeHeap"]   = ESP.getFreeHeap();
    attrs["wifiRSSI"]   = sdk.getNetworkRssi();
    attrs["uptime"]     = millis() / 1000;
    attrs["sdkVersion"] = 1.0;
    sdk.sendClientAttributes(attrs);
  }

  delay(10);
}

