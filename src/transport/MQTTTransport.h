// =========================================
// MQTTTransport.h
// =========================================

#pragma once

#include <WiFi.h>

#include <mqtt_client.h>

#include <ArduinoJson.h>

#include "../core/Config.h"
#include "../core/Callbacks.h"
#include "../core/Logger.h"
#include "../core/Types.h"

#include "ITransport.h"

class MQTTTransport : public ITransport {

public:

  MQTTTransport();

  // =========================================
  // CONFIGURE
  // =========================================

  void configure(
    SDKConfig* config,
    AttributeCallback attrCb,
    AttributeStringCallback attrStringCb = nullptr
  );

  // =========================================
  // LIFECYCLE
  // =========================================

  bool begin() override;

  void loop() override;

  bool connected() override;

  // =========================================
  // TRANSPORT INFO
  // =========================================

  bool isUsingWSS();

  bool isUsingMQTTS();

  String transportMode();

  // =========================================
  // TELEMETRY
  // =========================================

  bool sendTelemetry(
    const String& payload
  ) override;

  // =========================================
  // CLIENT ATTRIBUTES
  // =========================================

  bool sendClientAttribute(
    const String& key,
    float value
  ) override;

  bool sendClientAttributes(
    const String& payload
  ) override;

  // =========================================
  // SHARED ATTRIBUTES
  // =========================================

  void requestAttributes() override;

  void requestAttributes(
    const String& keys
  ) override;

  // =========================================
  // RPC
  // =========================================

  bool sendRPCResponse(
    const String& payload
  ) override;

  // =========================================
  // RPC CALLBACK
  // =========================================

  void onRPC(
    RPCCallback cb
  );

private:

  // =========================================
  // CONFIG
  // =========================================

  SDKConfig* _config = nullptr;

  AttributeCallback _attrCb = nullptr;

  AttributeStringCallback _attrStringCb = nullptr;

  RPCCallback _rpcCb = nullptr;

  // =========================================
  // MQTT CLIENT
  // =========================================

  esp_mqtt_client_handle_t client =
    nullptr;

  bool reconnectRequested =
    false;

  // =========================================
  // STATE
  // =========================================

  bool mqttConnected =
    false;

  bool usingWSS =
    true;

  /** Toggle after a failed Wi-Fi connect to try the other port (WSS ↔ MQTTS). */
  bool wifiUseAlternateTransport =
    false;

  unsigned long reconnectEarliestMs =
    0;

  int reconnectAttempt =
    0;

  bool failureArmed =
    false;

  int subSharedId =
    -1;

  int subResponseId =
    -1;

  bool sharedSubAck =
    false;

  bool responseSubAck =
    false;

  unsigned long sharedPullAt =
    0;

  bool sharedRetried =
    false;

  bool sharedDataSeen =
    false;

  struct SeenShared {
    char key[40];
    char value[48];
    unsigned long at;
  };

  SeenShared seenShared[8];

  int seenSharedCount =
    0;

  String lastRPCRequestId =
    "";

  // =========================================
  // INTERNALS
  // =========================================

  void connectClient();

  String topic(
    const String& suffix
  );

  String buildBrokerURI();

  bool publish(
    const String& topic,
    const String& payload
  );

  void armReconnect(
    bool countFailure,
    bool flipTransport
  );

  bool duplicateShared(
    const char* key,
    const String& value
  );

  void maybePullShared();

  // =========================================
  // EVENTS
  // =========================================

  static void mqttEventHandlerStatic(
    void* handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void* event_data
  );

  void mqttEventHandler(
    esp_mqtt_event_handle_t event
  );
};