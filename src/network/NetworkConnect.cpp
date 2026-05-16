#include "NetworkConnect.h"

#include "LtePppNetwork.h"

#include "../core/Logger.h"

#include <WiFi.h>

static bool wifiConnect(SDKConfig& config) {

  Logger::info("Connecting WiFi...");

  WiFi.mode(WIFI_STA);

  WiFi.begin(
    config.wifiSSID.c_str(),
    config.wifiPassword.c_str()
  );

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Logger::info("WiFi connected");

  Logger::info(
    WiFi.localIP().toString()
  );

  return true;
}

bool networkConnect(SDKConfig& config) {

  if (config.networkMode == NetworkMode::LtePpp) {
    return ltePppConnect(config);
  }

  return wifiConnect(config);
}

void networkMaintain(SDKConfig& config) {

  if (config.networkMode == NetworkMode::LtePpp) {
    ltePppMaintain();
  }
}

bool networkLinkUp(const SDKConfig& config) {

  if (config.networkMode == NetworkMode::LtePpp) {
    return ltePppLinkUp();
  }

  return WiFi.status() == WL_CONNECTED;
}

int networkSignalRssi(const SDKConfig& config) {

  if (config.networkMode == NetworkMode::LtePpp) {
    return ltePppSignalRssi();
  }

  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.RSSI();
  }

  return 0;
}
