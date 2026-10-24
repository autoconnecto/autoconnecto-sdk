// =============================================================
// WifiJoinStrongest_mqtt
//
// On a mesh, the default join scans every channel and picks the
// strongest access point (config.wifiJoinStrongest = true).
// Set it false to keep the first match the radio finds.
//
// enableSerialLogs is false in this sketch. The join line still
// prints, because onWifiJoin is not gated by the log switch.
//   [WIFI] joined bssid=.. ch=N rssi=-N
// A refused token prints from onMqttRefuse. The sketch does not
// print the Wi-Fi password or the device token.
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
  config.enableSerialLogs = false;

  sdk.onWifiJoin([](const char* bssid, int channel, int rssi) {
    Serial.printf("[WIFI] joined bssid=%s ch=%d rssi=%d\n", bssid, channel, rssi);
  });
  sdk.onMqttRefuse([](int code) {
    Serial.printf("[MQTT] broker refused the token (code %d)\n", code);
  });
  sdk.begin(config);
  Serial.println("[SDK] WifiJoinStrongest_mqtt started");
}

void loop() {
  sdk.loop();
  delay(10);
}
