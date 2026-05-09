// =========================================
// AutoconnectoSDK.h
// =========================================

#pragma once

#include <WiFi.h>

#include <ArduinoJson.h>

#include "core/Callbacks.h"
#include "core/ConnectionManager.h"
#include "core/Logger.h"
#include "core/Config.h"

class AutoconnectoSDK {

public:

  // =====================================
  // LIFECYCLE
  // =====================================

  void begin(
    SDKConfig config
  );

  void loop();

  bool connected();

  // =====================================
  // TRANSPORT INFO
  // =====================================

  bool isUsingWSS();

  bool isUsingMQTTS();

  String activeTransport();

  // =====================================
  // CALLBACKS
  // =====================================

  void onAttributeUpdate(
    AttributeCallback cb
  );

  void onConnect(
    ConnectionCallback cb
  );

  void onDisconnect(
    ConnectionCallback cb
  );

  void onRPC(
    RPCCallback cb
  );

  // =====================================
  // TELEMETRY
  // =====================================

  bool sendTelemetryJson(
    const String& json
  );

  bool sendTelemetry(
    JsonDocument& doc
  );

  bool sendTelemetry(
    const String& key,
    float value
  );

  bool sendTelemetry(
    const String& key1,
    float value1,
    const String& key2,
    float value2
  );

  bool sendTelemetry(
    const String& key1,
    float value1,
    const String& key2,
    float value2,
    const String& key3,
    float value3
  );

  // =====================================
  // CLIENT ATTRIBUTES
  // =====================================

  bool sendClientAttribute(
    const String& key,
    float value
  );

  bool sendClientAttributes(
    JsonDocument& doc
  );

  // =====================================
  // SHARED ATTRIBUTES
  // =====================================

  void requestSharedAttributes();

  void requestSharedAttributes(
    const String& keys
  );

  // =====================================
  // RPC
  // =====================================

  bool replyRPC(
    JsonDocument& doc
  );

  bool replyRPC(
    bool success
  );

private:

  // =====================================
  // CONFIG
  // =====================================

  SDKConfig _config;

  // =====================================
  // CONNECTION MANAGER
  // =====================================

  ConnectionManager manager;

  // =====================================
  // CALLBACKS
  // =====================================

  AttributeCallback attrCb =
    nullptr;

  ConnectionCallback connectCb =
    nullptr;

  ConnectionCallback disconnectCb =
    nullptr;

  RPCCallback rpcCb =
    nullptr;

  // =====================================
  // INTERNAL STATE
  // =====================================

  bool lastConnectionState =
    false;

  String lastRPCRequestTopic =
    "";

  // =====================================
  // INTERNALS
  // =====================================

  void connectWiFi();
};