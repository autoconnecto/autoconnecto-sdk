// =============================================================
// WifiJoinStrongest_mqtt
//
// On a mesh, the default join scans every channel and picks the
// strongest access point (config.wifiJoinStrongest = true).
// Set it false to keep the first match the radio finds.
//
// Serial, when logs are on:
//   [WIFI] joined bssid=.. ch=N rssi=-N
// The sketch does not print the Wi-Fi password or the device token.
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

  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort = 8084;
  config.deviceToken = DEVICE_TOKEN;
  config.enableMQTT = true;
  config.enableWS = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = true;

  sdk.begin(config);
  Serial.println("[SDK] WifiJoinStrongest_mqtt started");
}

void loop() {
  sdk.loop();
  delay(10);
}
