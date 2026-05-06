#pragma once

#include "../transport/MQTTTransport.h"
#include "../transport/WSTransport.h"
#include "RuntimeState.h"
#include "core/Config.h"
class ConnectionManager {

public:

  void configure(
    SDKConfig* config,
    AttributeCallback attrCb
  );

  void begin();

  void loop();

  ITransport* activeTransport();

private:

  SDKConfig* _config = nullptr;

  MQTTTransport mqtt;
  WSTransport ws;

  RuntimeState state;
};