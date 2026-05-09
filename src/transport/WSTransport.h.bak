#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

#include "../core/Config.h"
#include "../core/Callbacks.h"
#include "../core/Logger.h"
#include "ITransport.h"

class WSTransport : public ITransport {

public:

  WSTransport();

  void configure(
    SDKConfig* config,
    AttributeCallback attrCb
  );

  bool begin() override;

  void loop() override;

  bool connected() override;

  bool sendTelemetry(
    const String& payload
  ) override;

  bool sendClientAttribute(
    const String& key,
    float value
  ) override;

  void requestAttributes() override;

private:

  WebSocketsClient ws;

  SDKConfig* _config = nullptr;

  AttributeCallback _attrCb = nullptr;

  bool wsConnected = false;
  bool wsAuthed = false;

  static WSTransport* instance;

  static void wsEventStatic(
    WStype_t type,
    uint8_t* payload,
    size_t length
  );

  void wsEvent(
    WStype_t type,
    uint8_t* payload,
    size_t length
  );
};