#include <AutoconnectoSDK.h>

AutoconnectoSDK sdk;

void setup() {

  SDKConfig config;

  config.wifiSSID = "wifi";
  config.wifiPassword = "pass";

  config.deviceToken = "token";

  config.mqttHost = "192.168.1.100";
  config.wsHost = "192.168.1.100";

  sdk.begin(config);
}

void loop() {

  sdk.loop();
}