#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "../core/Config.h"
#include "../core/Callbacks.h"
#include "../core/Logger.h"
#include "ITransport.h"

class MQTTTransport : public ITransport {

public:

  MQTTTransport();

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

  WiFiClient wifiClient;
  PubSubClient mqtt;

  SDKConfig* _config = nullptr;

  AttributeCallback _attrCb = nullptr;

  unsigned long lastReconnectAttempt = 0;

  String topic(const String& suffix);

  void reconnect();

  static MQTTTransport* instance;

  static void mqttCallbackStatic(
    char* topic,
    byte* payload,
    unsigned int length
  );

  void mqttCallback(
    char* topic,
    byte* payload,
    unsigned int length
  );
};