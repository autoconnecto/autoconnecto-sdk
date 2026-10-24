// =============================================================
// Attributes_RestoreOnBoot_mqtt
//
// On connect the SDK subscribes to shared attributes (retained push),
// then pulls them once. Apply the shared key "relay" to a GPIO and
// confirm with a client attribute. The same value is not applied again
// when the push, the response, and the pull carry it within 15 s.
// =============================================================

#include <AutoconnectoSDK.h>
#include "secrets.h"

AutoconnectoSDK sdk;

const int kRelayPin = 2;
const char* kKey = "relay";

void applyRelay(const String& key, float value) {
  if (key != kKey) return;
  const bool on = value >= 0.5f;
  digitalWrite(kRelayPin, on ? HIGH : LOW);
  sdk.sendClientAttribute(kKey, on ? 1.0f : 0.0f);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(kRelayPin, OUTPUT);
  digitalWrite(kRelayPin, LOW);

  SDKConfig config;
  config.wifiSSID = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;
  config.wifiJoinStrongest = true;
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort = 8084;
  config.deviceToken = DEVICE_TOKEN;
  config.sharedAttributeKeys = kKey;
  config.enableSerialLogs = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;

  sdk.onAttributeUpdate(applyRelay);
  sdk.begin(config);
  Serial.println("[SDK] Attributes_RestoreOnBoot_mqtt started");
}

void loop() {
  sdk.loop();
  delay(10);
}
