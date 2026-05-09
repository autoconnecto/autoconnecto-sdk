// =========================================
// AutoconnectoSDK.cpp
// =========================================

#include "AutoconnectoSDK.h"

// =========================================
// WIFI
// =========================================

void AutoconnectoSDK::connectWiFi() {

  Logger::info(
    "Connecting WiFi..."
  );

  WiFi.begin(
    _config.wifiSSID.c_str(),
    _config.wifiPassword.c_str()
  );

  while (
    WiFi.status() != WL_CONNECTED
  ) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Logger::info(
    "WiFi connected"
  );

  Logger::info(
    WiFi.localIP().toString()
  );
}

// =========================================
// BEGIN
// =========================================

void AutoconnectoSDK::begin(
  SDKConfig config
) {

  _config = config;



  connectWiFi();

  manager.configure(
    &_config,
    attrCb
  );

  manager.onRPC(
    rpcCb
  );

  manager.begin();
}

// =========================================
// LOOP
// =========================================

void AutoconnectoSDK::loop() {

  manager.loop();

  bool currentState =
    manager.activeTransport() != nullptr;

  // =====================================
  // CONNECTED
  // =====================================

  if (
    currentState &&
    !lastConnectionState
  ) {

    lastConnectionState =
      true;

    Logger::info(
      "SDK Connected"
    );

    if (connectCb) {

      connectCb(true);
    }
  }

  // =====================================
  // DISCONNECTED
  // =====================================

  if (
    !currentState &&
    lastConnectionState
  ) {

    lastConnectionState =
      false;

    Logger::warn(
      "SDK Disconnected"
    );

    if (disconnectCb) {

      disconnectCb(false);
    }
  }
}

// =========================================
// CONNECTED
// =========================================

bool AutoconnectoSDK::connected() {

  return
    manager.activeTransport()
    != nullptr;
}

// =========================================
// TRANSPORT INFO
// =========================================

bool AutoconnectoSDK::isUsingWSS() {

  return
    manager.transportMode()
    == "WSS";
}

bool AutoconnectoSDK::isUsingMQTTS() {

  return
    manager.transportMode()
    == "MQTTS";
}

String AutoconnectoSDK::activeTransport() {

  return
    manager.transportMode();
}

// =========================================
// CALLBACKS
// =========================================

void AutoconnectoSDK::onAttributeUpdate(
  AttributeCallback cb
) {

  attrCb = cb;
}

void AutoconnectoSDK::onConnect(
  ConnectionCallback cb
) {

  connectCb = cb;
}

void AutoconnectoSDK::onDisconnect(
  ConnectionCallback cb
) {

  disconnectCb = cb;
}

void AutoconnectoSDK::onRPC(
  RPCCallback cb
) {

  rpcCb = cb;
}

// =========================================
// TELEMETRY JSON
// =========================================

bool AutoconnectoSDK::sendTelemetryJson(
  const String& json
) {

  auto transport =
    manager.activeTransport();

  if (!transport) {

    return false;
  }

  return transport
    ->sendTelemetry(json);
}

// =========================================
// TELEMETRY DOC
// =========================================

bool AutoconnectoSDK::sendTelemetry(
  JsonDocument& doc
) {

  String out;

  serializeJson(
    doc,
    out
  );

  return sendTelemetryJson(
    out
  );
}

// =========================================
// TELEMETRY SINGLE
// =========================================

bool AutoconnectoSDK::sendTelemetry(
  const String& key,
  float value
) {

  StaticJsonDocument<128> doc;

  doc[key] = value;

  return sendTelemetry(doc);
}

// =========================================
// TELEMETRY DOUBLE
// =========================================

bool AutoconnectoSDK::sendTelemetry(
  const String& key1,
  float value1,
  const String& key2,
  float value2
) {

  StaticJsonDocument<256> doc;

  doc[key1] = value1;

  doc[key2] = value2;

  return sendTelemetry(doc);
}

// =========================================
// TELEMETRY TRIPLE
// =========================================

bool AutoconnectoSDK::sendTelemetry(
  const String& key1,
  float value1,
  const String& key2,
  float value2,
  const String& key3,
  float value3
) {

  StaticJsonDocument<384> doc;

  doc[key1] = value1;

  doc[key2] = value2;

  doc[key3] = value3;

  return sendTelemetry(doc);
}

// =========================================
// CLIENT ATTRIBUTE
// =========================================

bool AutoconnectoSDK::sendClientAttribute(
  const String& key,
  float value
) {

  auto transport =
    manager.activeTransport();

  if (!transport) {

    return false;
  }

  return transport
    ->sendClientAttribute(
      key,
      value
    );
}

// =========================================
// CLIENT ATTRIBUTES
// =========================================

bool AutoconnectoSDK::sendClientAttributes(
  JsonDocument& doc
) {

  auto transport =
    manager.activeTransport();

  if (!transport) {

    return false;
  }

  String out;

  serializeJson(
    doc,
    out
  );

  return transport
    ->sendClientAttributes(
      out
    );
}

// =========================================
// SHARED ATTRIBUTES
// =========================================

void AutoconnectoSDK::requestSharedAttributes() {

  auto transport =
    manager.activeTransport();

  if (!transport) {

    return;
  }

  transport
    ->requestAttributes();
}

void AutoconnectoSDK::requestSharedAttributes(
  const String& keys
) {

  auto transport =
    manager.activeTransport();

  if (!transport) {

    return;
  }

  transport
    ->requestAttributes(
      keys
    );
}

// =========================================
// RPC RESPONSE
// =========================================

bool AutoconnectoSDK::replyRPC(
  JsonDocument& doc
) {

  auto transport =
    manager.activeTransport();

  if (!transport) {

    return false;
  }

  String out;

  serializeJson(
    doc,
    out
  );

  return transport
    ->sendRPCResponse(
      out
    );
}

bool AutoconnectoSDK::replyRPC(
  bool success
) {

  StaticJsonDocument<128> doc;

  doc["success"] =
    success;

  return replyRPC(doc);
}