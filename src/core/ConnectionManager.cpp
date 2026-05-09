#include "ConnectionManager.h"

#include "AutoconnectoSDK.h"
#include "core/Config.h"

// =========================================
// CONFIGURE
// =========================================

void ConnectionManager::configure(
  SDKConfig* config,
  AttributeCallback attrCb
) {

  _config = config;

  mqtt.configure(
    config,
    attrCb
  );
}

// =========================================
// BEGIN
// =========================================

void ConnectionManager::begin() {

  mqtt.begin();
}

// =========================================
// LOOP
// =========================================

void ConnectionManager::loop() {

  mqtt.loop();

  // =====================================
  // ACTIVE TRANSPORT STATE
  // =====================================

  if (mqtt.connected()) {

    state.activeTransport =
      TRANSPORT_MQTT;

  } else {

    state.activeTransport =
      TRANSPORT_NONE;
  }
}

// =========================================
// ACTIVE TRANSPORT
// =========================================

ITransport*
ConnectionManager::activeTransport() {

  if (
    state.activeTransport ==
    TRANSPORT_MQTT
  ) {

    return &mqtt;
  }

  return nullptr;
}

// =========================================
// TRANSPORT MODE
// =========================================

String ConnectionManager::transportMode() {

  if (!mqtt.connected()) {

    return "NONE";
  }

  if (mqtt.isUsingWSS()) {

    return "WSS";
  }

  return "MQTTS";
}

// =========================================
// RPC
// =========================================

void ConnectionManager::onRPC(
  RPCCallback cb
) {

  mqtt.onRPC(cb);
}