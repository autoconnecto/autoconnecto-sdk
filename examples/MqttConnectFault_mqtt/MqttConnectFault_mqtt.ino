// =============================================================
// MqttConnectFault_mqtt
//
// Software fault injection. No broker socket is opened for the
// first two attempts. Each attempt flips WSS/MQTTS and arms
// backoff. Serial, with logs on:
//   MQTT connect failed — trying alternate transport
//   MQTT fault injection delayMs=...
//
// Wi-Fi timeout is config.wifiConnectTimeoutMs (default 15 s).
// To check that clock, set wifiSSID to a name that is not here
// and watch for "WiFi connect timed out" within that budget.
// This sketch uses the real SSID so the third attempt can connect.
// =============================================================

#include <AutoconnectoSDK.h>
#include "secrets.h"

AutoconnectoSDK sdk;

void setup() {
  Serial.begin(115200);
  delay(1500);

  SDKConfig config;
  config.wifiSSID = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;
  config.wifiJoinStrongest = true;
  config.wifiConnectTimeoutMs = 15000;
  config.injectConnectFailures = 2;

  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort = 8084;
  config.deviceToken = DEVICE_TOKEN;
  config.enableMQTT = true;
  config.enableWS = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = true;

  sdk.onMqttRefuse([](int code) {
    Serial.printf("[MQTT] broker refused the token (code %d)\n", code);
  });
  sdk.begin(config);
  Serial.println("[SDK] MqttConnectFault_mqtt started");
}

void loop() {
  sdk.loop();
  delay(10);
}
