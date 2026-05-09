#pragma once

#include "../transport/MQTTTransport.h"

#include "RuntimeState.h"
#include "core/Config.h"

class ConnectionManager {

public:

  // =========================================
  // CONFIGURE
  // =========================================

  void configure(
    SDKConfig* config,
    AttributeCallback attrCb
  );

  // =========================================
  // LIFECYCLE
  // =========================================

  void begin();

  void loop();

  // =========================================
  // ACTIVE TRANSPORT
  // =========================================

  ITransport* activeTransport();

  // =========================================
  // TRANSPORT INFO
  // =========================================

  String transportMode();

  // =========================================
  // RPC
  // =========================================

  void onRPC(
    RPCCallback cb
  );

private:

  SDKConfig* _config = nullptr;

  // =========================================
  // UNIFIED MQTT TRANSPORT
  // =========================================

  MQTTTransport mqtt;

  RuntimeState state;
};