#include "MQTTTransport.h"

MQTTTransport* MQTTTransport::instance = nullptr;

MQTTTransport::MQTTTransport()
  : mqtt(wifiClient) {

  instance = this;
}

void MQTTTransport::configure(
  SDKConfig* config,
  AttributeCallback attrCb
) {
  _config = config;
  _attrCb = attrCb;
}

bool MQTTTransport::begin() {

  mqtt.setServer(
    _config->mqttHost.c_str(),
    _config->mqttPort
  );

  mqtt.setCallback(mqttCallbackStatic);

  mqtt.setKeepAlive(30);

  mqtt.setBufferSize(1024);

  return true;
}

void MQTTTransport::loop() {

  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop();
}

bool MQTTTransport::connected() {
  return mqtt.connected();
}

void MQTTTransport::reconnect() {

  unsigned long now = millis();

  if (now - lastReconnectAttempt < 3000) {
    return;
  }

  lastReconnectAttempt = now;

  Logger::info("MQTT connecting...");

  if (mqtt.connect(_config->deviceToken.c_str())) {

    Logger::info("MQTT connected");

    mqtt.subscribe(
      topic("attributes/shared").c_str()
    );

  } else {

    Logger::warn(
      "MQTT failed rc=" + String(mqtt.state())
    );
  }
}

bool MQTTTransport::sendTelemetry(
  const String& payload
) {

  return mqtt.publish(
    topic("telemetry").c_str(),
    payload.c_str()
  );
}

bool MQTTTransport::sendClientAttribute(
  const String& key,
  float value
) {

  StaticJsonDocument<128> doc;

  doc[key] = value;

  String out;

  serializeJson(doc, out);

  return mqtt.publish(
    topic("attributes/client").c_str(),
    out.c_str()
  );
}

void MQTTTransport::requestAttributes() {

  StaticJsonDocument<256> doc;

  doc["sharedKeys"] =
    "channel1,channel2,volume,"
    "limitVoltage1,limitVoltage2,limitVoltage3";

  String out;

  serializeJson(doc, out);

  mqtt.publish(
    topic("attributes/shared/request").c_str(),
    out.c_str()
  );
}

String MQTTTransport::topic(
  const String& suffix
) {

  return
    "devices/" +
    _config->deviceToken +
    "/" +
    suffix;
}

void MQTTTransport::mqttCallbackStatic(
  char* topic,
  byte* payload,
  unsigned int length
) {

  if (instance) {
    instance->mqttCallback(
      topic,
      payload,
      length
    );
  }
}

void MQTTTransport::mqttCallback(
  char* topicStr,
  byte* payload,
  unsigned int length
) {

  String msg;

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Logger::info(
    "MQTT RX: " + msg
  );

  StaticJsonDocument<512> doc;

  if (deserializeJson(doc, msg)) {
    Logger::warn("MQTT JSON parse failed");
    return;
  }

  String topicName = String(topicStr);

  if (
    topicName.endsWith("/attributes/shared") ||
    topicName.endsWith("/attributes/shared/response")
  ) {

    for (JsonPair kv : doc.as<JsonObject>()) {

      if (_attrCb) {

        _attrCb(
          kv.key().c_str(),
          kv.value().as<float>()
        );
      }
    }
  }
}