#include "LtePppNetwork.h"

#include "../core/AutoconnectoFeatures.h"
#include "../core/Logger.h"

#if defined(AUTOCONNECTO_ENABLE_LTE_PPP) && AUTOCONNECTO_ENABLE_LTE_PPP

#include <Network.h>
#include <PPP.h>

static bool g_lteDataUp = false;

static void onPppEvent(
  arduino_event_id_t event,
  arduino_event_info_t info
) {
  (void)info;

  switch (event) {
    case ARDUINO_EVENT_PPP_GOT_IP:
      g_lteDataUp = true;
      Logger::info("PPP Got IP");
      break;
    case ARDUINO_EVENT_PPP_LOST_IP:
    case ARDUINO_EVENT_PPP_DISCONNECTED:
      g_lteDataUp = false;
      Logger::warn("PPP link down");
      break;
    default:
      break;
  }
}

bool ltePppConnect(SDKConfig& config) {

  Logger::info("Connecting LTE (PPP / EC200)...");

  g_lteDataUp = false;

  Network.onEvent(onPppEvent);

  PPP.setApn(config.lteApn.c_str());

  if (config.ltePin.length() > 0) {
    PPP.setPin(config.ltePin.c_str());
  }

  if (config.lteResetPin >= 0) {
    PPP.setResetPin(
      config.lteResetPin,
      config.lteResetActiveLow,
      config.lteResetDelayMs
    );
  }

  esp_modem_flow_control_t flow =
    ESP_MODEM_FLOW_CONTROL_NONE;

  if (
    config.lteRtsPin >= 0 &&
    config.lteCtsPin >= 0
  ) {
    flow = ESP_MODEM_FLOW_CONTROL_HW;
  }

  PPP.setPins(
    config.lteUartTx,
    config.lteUartRx,
    config.lteRtsPin,
    config.lteCtsPin,
    flow
  );

  if (!PPP.begin(PPP_MODEM_EC200)) {

    Logger::warn("PPP.begin failed");

    return false;
  }

  unsigned long start = millis();

  while (
    !PPP.attached() &&
    (millis() - start) < config.lteAttachTimeoutMs
  ) {
    delay(100);
  }

  if (!PPP.attached()) {

    Logger::warn("LTE network attach timeout");

    return false;
  }

  Logger::info(
    "LTE registered: " + PPP.operatorName()
  );

  Logger::info(
    "RSSI: " + String(PPP.RSSI())
  );

  PPP.mode(ESP_MODEM_MODE_CMUX);

  if (
    !PPP.waitStatusBits(
      ESP_NETIF_CONNECTED_BIT,
      60000
    )
  ) {

    Logger::warn("PPP data / IP timeout");

    return false;
  }

  g_lteDataUp = true;

  Logger::info("LTE PPP data connected");

  return true;
}

bool ltePppLinkUp() {

  return g_lteDataUp && PPP.connected();
}

void ltePppMaintain() {

  if (ltePppLinkUp()) {
    return;
  }

  if (!PPP.attached()) {
    return;
  }

  PPP.mode(ESP_MODEM_MODE_CMUX);

  if (
    PPP.waitStatusBits(
      ESP_NETIF_CONNECTED_BIT,
      30000
    )
  ) {
    g_lteDataUp = true;
  }
}

int ltePppSignalRssi() {

  return PPP.RSSI();
}

#else

bool ltePppConnect(SDKConfig& config) {

  (void)config;

  Logger::warn(
    "LTE PPP not compiled in. Add -DAUTOCONNECTO_ENABLE_LTE_PPP=1 "
    "to build_opt.h (ESP32 core 3.x PPP library required)."
  );

  return false;
}

bool ltePppLinkUp() {
  return false;
}

void ltePppMaintain() {}

int ltePppSignalRssi() {
  return 0;
}

#endif
