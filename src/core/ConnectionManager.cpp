#include "ConnectionManager.h"
#include "AutoconnectoSDK.h"
#include "core/Config.h"
void ConnectionManager::configure(
  SDKConfig* config,
  AttributeCallback attrCb
) {

  _config = config;

  mqtt.configure(config, attrCb);

  ws.configure(config, attrCb);
}

void ConnectionManager::begin() {

  if (_config->enableMQTT) {
    mqtt.begin();
  }

  if (_config->enableWS) {
    ws.begin();
  }
}

void ConnectionManager::loop() {

  if (_config->enableWS) {
    ws.loop();
  }

  if (_config->enableMQTT) {
    mqtt.loop();
  }

  if (
    _config->preferWS &&
    ws.connected()
  ) {

    state.activeTransport = TRANSPORT_WS;

    return;
  }

  if (
    _config->mqttFallback &&
    mqtt.connected()
  ) {

    state.activeTransport = TRANSPORT_MQTT;

    return;
  }

  state.activeTransport = TRANSPORT_NONE;
}

ITransport* ConnectionManager::activeTransport() {

  switch(state.activeTransport) {

    case TRANSPORT_WS:
      return &ws;

    case TRANSPORT_MQTT:
      return &mqtt;

    default:
      return nullptr;
  }
}