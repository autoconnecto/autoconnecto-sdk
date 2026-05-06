#pragma once

#include <WiFi.h>
#include <ArduinoJson.h>

#include "core/Callbacks.h"
#include "core/ConnectionManager.h"
#include "core/Logger.h"
#include "core/Config.h"

class AutoconnectoSDK {

public:

  void begin(SDKConfig config);

  void loop();

  void onAttributeUpdate(
    AttributeCallback cb
  );

  bool connected();

  bool sendTelemetryJson(
    const String& json
  );

  bool sendTelemetry(
    JsonDocument& doc
  );

  bool sendClientAttribute(
    const String& key,
    float value
  );

private:

  SDKConfig _config;

  ConnectionManager manager;

  AttributeCallback attrCb = nullptr;

  void connectWiFi();
};