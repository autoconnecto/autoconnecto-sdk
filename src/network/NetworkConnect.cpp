#include "../core/Esp32Only.h"
#if defined(AUTOCONNECTO_ESP32_CLIENT)

#include "NetworkConnect.h"

#include "LtePppNetwork.h"

#include "../core/Logger.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <string.h>

static uint8_t lastBssid[6];
static bool haveAp = false;
static int lastChannel = -1;
static WifiJoinInfo lastJoin = {};
static WifiJoinCallback joinCb = nullptr;

void networkOnWifiJoin(WifiJoinCallback cb) {
  joinCb = cb;
}

bool networkLastWifiJoin(WifiJoinInfo& out) {
  if (!lastJoin.valid) return false;
  out = lastJoin;
  return true;
}

static void applyJoinPreference(const SDKConfig& config) {
  WiFi.persistent(false);
  if (config.wifiJoinStrongest) {
    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  } else {
    WiFi.setScanMethod(WIFI_FAST_SCAN);
  }
}

static void patchStoredJoin(const SDKConfig& config) {
  wifi_config_t conf;
  if (esp_wifi_get_config(WIFI_IF_STA, &conf) != ESP_OK) return;
  conf.sta.scan_method = config.wifiJoinStrongest
    ? WIFI_ALL_CHANNEL_SCAN
    : WIFI_FAST_SCAN;
  if (config.wifiJoinStrongest) {
    conf.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
  }
  esp_wifi_set_config(WIFI_IF_STA, &conf);
}

static void logWifiAp(const char* verb) {
  if (WiFi.status() != WL_CONNECTED) return;
  const uint8_t* bssid = WiFi.BSSID();
  if (!bssid) return;
  const int channel = WiFi.channel();
  const bool changed =
    !haveAp ||
    memcmp(lastBssid, bssid, 6) != 0 ||
    lastChannel != channel;
  if (!changed && verb == nullptr) return;
  memcpy(lastBssid, bssid, 6);
  lastChannel = channel;
  haveAp = true;
  const String bssidText = WiFi.BSSIDstr();
  const int rssi = WiFi.RSSI();
  memset(&lastJoin, 0, sizeof(lastJoin));
  lastJoin.valid = true;
  lastJoin.channel = channel;
  lastJoin.rssi = rssi;
  strncpy(lastJoin.bssid, bssidText.c_str(), sizeof(lastJoin.bssid) - 1);
  if (joinCb) joinCb(lastJoin.bssid, channel, rssi);
  Logger::note(
    String("[WIFI] ") +
    (verb ? verb : "joined") +
    " bssid=" + bssidText +
    " ch=" + String(channel) +
    " rssi=" + String(rssi)
  );
}

static bool wifiConnect(SDKConfig& config) {

  WiFi.mode(WIFI_STA);
  applyJoinPreference(config);

  if (WiFi.status() == WL_CONNECTED) {
    Logger::info("WiFi already connected");
    Logger::info(WiFi.localIP().toString());
    logWifiAp("joined");
    return true;
  }

  Logger::info("Connecting WiFi...");

  if (config.wifiSSID.length() == 0) {
    patchStoredJoin(config);
    WiFi.begin();
  } else {
    WiFi.begin(
      config.wifiSSID.c_str(),
      config.wifiPassword.c_str()
    );
  }

  const unsigned long timeoutMs =
    config.wifiConnectTimeoutMs == 0 ? 15000 : config.wifiConnectTimeoutMs;
  const unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - started >= timeoutMs) {
      Logger::warn("WiFi connect timed out");
      return false;
    }
    delay(250);
  }

  Logger::info("WiFi connected");
  Logger::info(WiFi.localIP().toString());
  logWifiAp("joined");

  // Let lwIP settle before the first TLS socket (avoids WSS connect timeouts).
  delay(500);

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
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    const uint8_t* bssid = WiFi.BSSID();
    const int channel = WiFi.channel();
    if (
      bssid &&
      haveAp &&
      (memcmp(lastBssid, bssid, 6) != 0 || lastChannel != channel)
    ) {
      logWifiAp("changed");
    }
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

#endif
