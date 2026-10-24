// =========================================
// MQTTTransport.cpp
// =========================================

#include "../core/Esp32Only.h"
#if defined(AUTOCONNECTO_ESP32_CLIENT)

#include "MQTTTransport.h"

#include <esp_system.h>

// =========================================
// GLOBAL INSTANCE
// =========================================

static MQTTTransport*
globalTransportInstance =
  nullptr;

// =========================================
// CONSTRUCTOR
// =========================================

MQTTTransport::MQTTTransport() {

  globalTransportInstance =
    this;
}

// =========================================
// CONFIGURE
// =========================================

void MQTTTransport::configure(
  SDKConfig* config,
  AttributeCallback attrCb,
  AttributeStringCallback attrStringCb
) {

  _config = config;

  _attrCb = attrCb;
  _attrStringCb = attrStringCb;
}

// =========================================
// BEGIN
// =========================================

bool MQTTTransport::begin() {

  connectClient();

  return true;
}

// =========================================
// LOOP
// =========================================

unsigned long MQTTTransport::armReconnect(
  bool countFailure,
  bool flipTransport
) {

  if (
    flipTransport &&
    _config &&
    _config->networkMode == NetworkMode::WiFi
  ) {

    wifiUseAlternateTransport =
      !wifiUseAlternateTransport;

    Logger::warn(
      wifiUseAlternateTransport
        ? "MQTT connect failed — trying alternate transport"
        : "MQTT connect failed — trying primary transport"
    );
  }

  if (countFailure && reconnectAttempt < 8) {
    reconnectAttempt++;
  }

  unsigned shift = reconnectAttempt > 0
    ? reconnectAttempt - 1
    : 0;
  if (shift > 5) shift = 5;

  unsigned long delayMs = 1000UL << shift;
  if (delayMs > 30000UL) delayMs = 30000UL;
  delayMs += esp_random() % 500UL;

  reconnectEarliestMs = millis() + delayMs;
  reconnectRequested = true;
  failureArmed = true;
  return delayMs;
}

bool MQTTTransport::duplicateShared(
  const char* key,
  const String& value
) {

  const unsigned long now = millis();
  const String shown = value.length() > 47
    ? value.substring(0, 47)
    : value;

  for (int i = 0; i < seenSharedCount; i++) {
    if (
      strcmp(seenShared[i].key, key) == 0 &&
      shown == seenShared[i].value &&
      now - seenShared[i].at < 15000UL
    ) {
      return true;
    }
  }

  SeenShared* slot = nullptr;
  if (seenSharedCount < 8) {
    slot = &seenShared[seenSharedCount++];
  } else {
    slot = &seenShared[0];
    for (int i = 1; i < 8; i++) {
      if (seenShared[i].at < slot->at) slot = &seenShared[i];
    }
  }

  strncpy(slot->key, key, sizeof(slot->key) - 1);
  slot->key[sizeof(slot->key) - 1] = '\0';
  strncpy(slot->value, shown.c_str(), sizeof(slot->value) - 1);
  slot->value[sizeof(slot->value) - 1] = '\0';
  slot->at = now;
  return false;
}

void MQTTTransport::maybePullShared() {

  if (
    !sharedSubAck ||
    !responseSubAck ||
    sharedPullAt != 0 ||
    sharedDataSeen
  ) {
    return;
  }

  const String keys =
    _config ? _config->sharedAttributeKeys : "";

  Logger::info(
    "MQTT pull shared attributes" +
    (keys.length() ? " keys=" + keys : " (all)")
  );

  requestAttributes(keys);
  sharedPullAt = millis();
  sharedRetried = false;
  sharedDataSeen = false;
}

void MQTTTransport::loop() {

  if (
    sharedPullDueAt != 0 &&
    (long)(millis() - sharedPullDueAt) >= 0
  ) {
    sharedPullDueAt = 0;
    maybePullShared();
  }

  if (
    mqttConnected &&
    sharedPullAt != 0 &&
    !sharedDataSeen &&
    !sharedRetried &&
    millis() - sharedPullAt >= 6000UL
  ) {

    sharedRetried = true;
    Logger::info("MQTT pull shared attributes retry");
    const String keys =
      _config ? _config->sharedAttributeKeys : "";
    requestAttributes(keys);
  }

  if (
    reconnectRequested &&
    !mqttConnected &&
    millis() >= reconnectEarliestMs
  ) {

    reconnectRequested = false;
    failureArmed = false;

    Logger::warn(
      "Reconnecting MQTT..."
    );

    connectClient();
  }
}

// =========================================
// CONNECTED
// =========================================

bool MQTTTransport::connected() {

  return mqttConnected;
}

// =========================================
// TRANSPORT INFO
// =========================================

bool MQTTTransport::isUsingWSS() {

  return usingWSS;
}

bool MQTTTransport::isUsingMQTTS() {

  return !usingWSS;
}

String MQTTTransport::transportMode() {

  return usingWSS
    ? "WSS"
    : "MQTTS";
}

// =========================================
// CONNECT CLIENT
// =========================================

void MQTTTransport::connectClient() {

  if (!_config) {

    Logger::warn(
      "MQTT config missing"
    );

    return;
  }

  if (_config->injectConnectFailures > 0) {
    _config->injectConnectFailures--;
    const unsigned long delayMs = armReconnect(true, true);
    const bool nextWss = wifiUseAlternateTransport
      ? !_config->enableWS
      : _config->enableWS;
    Logger::warn(
      String("MQTT fault injection delayMs=") +
      String(delayMs) +
      " next=" +
      (nextWss ? "WSS" : "MQTTS")
    );
    return;
  }

  String brokerURI =
    buildBrokerURI();

  Logger::info(
    "Broker URI: " +
    brokerURI
  );

  esp_mqtt_client_config_t mqtt_cfg =
    {};

  mqtt_cfg.broker.address.uri =
    brokerURI.c_str();

  mqtt_cfg.credentials.username =
    _config->deviceToken.c_str();

  mqtt_cfg.credentials.client_id =
    _config->deviceToken.c_str();

  mqtt_cfg.network.timeout_ms = 45000;
  mqtt_cfg.network.disable_auto_reconnect = true;
  mqtt_cfg.session.disable_clean_session = true;
  mqtt_cfg.session.keepalive = 60;

  // =====================================
  // TLS
  // =====================================

  if (
    _config->allowInsecureTLS
  ) {

    mqtt_cfg.broker.verification
      .skip_cert_common_name_check =
        true;

    mqtt_cfg.broker.verification
      .certificate = nullptr;

  } else {

    mqtt_cfg.broker.verification
      .certificate =
        _config->rootCA;
  }

  // =====================================
  // CLEANUP OLD CLIENT
  // =====================================

  if (client) {

    esp_mqtt_client_stop(
      client
    );

    esp_mqtt_client_destroy(
      client
    );

    client = nullptr;
  }

  // =====================================
  // CREATE CLIENT
  // =====================================

  client =
    esp_mqtt_client_init(
      &mqtt_cfg
    );

  if (!client) {

    Logger::warn(
      "MQTT client init failed"
    );

    return;
  }

  // =====================================
  // REGISTER EVENTS
  // =====================================

  esp_mqtt_client_register_event(
    client,
    MQTT_EVENT_ANY,
    mqttEventHandlerStatic,
    nullptr
  );

  // =====================================
  // START
  // =====================================

  esp_err_t err =
    esp_mqtt_client_start(
      client
    );

  if (err != ESP_OK) {

    Logger::warn(
      "MQTT start failed"
    );

    Logger::warn(
      String(err)
    );
  }
}

// =========================================
// BUILD URI
// =========================================

String MQTTTransport::buildBrokerURI() {

  // LTE PPP: MQTTS only (cellular).
  if (
    _config->networkMode ==
    NetworkMode::LtePpp
  ) {

    usingWSS = false;

    return
      "mqtts://" +
      _config->mqttHost +
      ":" +
      String(_config->mqttPort);
  }

  // WiFi: WSS or MQTTS; flip on connect timeout (wifiUseAlternateTransport).
  const bool useWss =
    wifiUseAlternateTransport
      ? !_config->enableWS
      : _config->enableWS;

  if (useWss) {

    usingWSS = true;

    if (_config->mqttUseTls) {

      return
        "wss://" +
        _config->mqttHost +
        ":" +
        String(_config->wssPort) +
        "/mqtt";
    }

    return
      "ws://" +
      _config->mqttHost +
      ":" +
      String(_config->wssPort) +
      "/mqtt";
  }

  usingWSS = false;

  if (_config->mqttUseTls) {

    return
      "mqtts://" +
      _config->mqttHost +
      ":" +
      String(_config->mqttPort);
  }

  return
    "mqtt://" +
    _config->mqttHost +
    ":" +
    String(_config->mqttPort);
}

// =========================================
// TOPIC
// =========================================

String MQTTTransport::topic(
  const String& suffix
) {

  return
    "devices/" +
    _config->deviceToken +
    "/" +
    suffix;
}

// =========================================
// PUBLISH
// =========================================

bool MQTTTransport::publish(
  const String& topicName,
  const String& payload
) {

  if (
    !client ||
    !mqttConnected
  ) {

    return false;
  }

  int msgId =
    esp_mqtt_client_publish(
      client,
      topicName.c_str(),
      payload.c_str(),
      0,
      1,
      0
    );

  return msgId != -1;
}

// =========================================
// TELEMETRY
// =========================================

bool MQTTTransport::sendTelemetry(
  const String& payload
) {

  return publish(
    topic("telemetry"),
    payload
  );
}

// =========================================
// CLIENT ATTRIBUTE
// =========================================

bool MQTTTransport::sendClientAttribute(
  const String& key,
  float value
) {

  StaticJsonDocument<128> doc;

  doc[key] = value;

  String out;

  serializeJson(doc, out);

  return sendClientAttributes(
    out
  );
}

bool MQTTTransport::sendClientAttributes(
  const String& payload
) {

  return publish(
    topic("attributes/client"),
    payload
  );
}

// =========================================
// REQUEST ATTRIBUTES
// =========================================

void MQTTTransport::requestAttributes() {

  requestAttributes("");
}

void MQTTTransport::requestAttributes(
  const String& keys
) {

  if (
    sharedDataSeen &&
    millis() - sharedDataAt < 15000UL
  ) {
    Logger::info(
      "MQTT shared request skipped — snapshot already received"
    );
    return;
  }

  if (
    keys == lastSharedRequestKeys &&
    lastSharedRequestAt != 0 &&
    millis() - lastSharedRequestAt < 2000UL
  ) {
    Logger::info(
      "MQTT shared request skipped — same keys just sent"
    );
    return;
  }

  lastSharedRequestAt = millis();
  lastSharedRequestKeys = keys;

  StaticJsonDocument<256> doc;

  if (keys.length()) {

    doc["keys"] = keys;
    doc["sharedKeys"] = keys;
  }

  doc["requestId"] =
    millis();

  String out;

  serializeJson(doc, out);

  const String reqTopic =
    topic("attributes/shared/request");

  Logger::info(
    "MQTT TX Topic: " +
    reqTopic
  );

  Logger::info(
    "MQTT TX Payload: " +
    out
  );

  if (
    !publish(
      reqTopic,
      out
    )
  ) {

    Logger::warn(
      "MQTT TX failed — not connected"
    );
  }
}

// =========================================
// RPC RESPONSE
// =========================================

bool MQTTTransport::sendRPCResponse(
  const String& payload
) {

  if (lastRPCRequestId == "") {

    Logger::warn(
      "No RPC request ID"
    );

    return false;
  }

  // Must match the backend ACK consumer subscription: devices/+/rpc/response/+
  String responseTopic =
    topic("rpc/response/") +
    lastRPCRequestId;

  return publish(
    responseTopic,
    payload
  );
}
// =========================================
// RPC CALLBACK
// =========================================

void MQTTTransport::onRPC(
  RPCCallback cb
) {

  _rpcCb = cb;
}

void MQTTTransport::onMqttRefuse(
  MqttRefuseCallback cb
) {

  _refuseCb = cb;
}

// =========================================
// STATIC EVENT HANDLER
// =========================================

void MQTTTransport::mqttEventHandlerStatic(
  void* handler_args,
  esp_event_base_t base,
  int32_t event_id,
  void* event_data
) {

  if (
    globalTransportInstance
  ) {

    globalTransportInstance
      ->mqttEventHandler(
        (esp_mqtt_event_handle_t)
        event_data
      );
  }
}

// =========================================
// EVENT HANDLER
// =========================================

void MQTTTransport::mqttEventHandler(
  esp_mqtt_event_handle_t event
) {

  switch(event->event_id) {

    // =====================================
    // CONNECTED
    // =====================================

    case MQTT_EVENT_CONNECTED: {

      mqttConnected = true;
      reconnectAttempt = 0;
      failureArmed = false;
      sharedSubAck = false;
      responseSubAck = false;
      sharedPullAt = 0;
      sharedRetried = false;
      sharedDataSeen = false;

      Logger::info(
        "MQTT CONNECTED"
      );

      if (usingWSS) {

        Logger::info(
          _config && _config->mqttUseTls
            ? "Connected via WSS"
            : "Connected via WS"
        );

      } else {

        Logger::info(
          _config && _config->mqttUseTls
            ? "Connected via MQTTS"
            : "Connected via MQTT"
        );
      }

      // ===================================
      // SUBSCRIPTIONS
      // ===================================

      const String sharedTopic =
        topic("attributes/shared");

      const String responseTopic =
        topic("attributes/shared/response");

      const String rpcTopic =
        topic("rpc/request/+");

      Logger::info(
        "MQTT SUB " +
        sharedTopic +
        " (push/retain)"
      );

      Logger::info(
        "MQTT SUB " +
        responseTopic +
        " (pull response)"
      );

      Logger::info(
        "MQTT SUB " +
        rpcTopic
      );

      subSharedId = esp_mqtt_client_subscribe(
        client,
        sharedTopic.c_str(),
        1
      );

      subResponseId = esp_mqtt_client_subscribe(
        client,
        responseTopic.c_str(),
        1
      );

      esp_mqtt_client_subscribe(
        client,
        rpcTopic.c_str(),
        1
      );

      if (subSharedId < 0 || subResponseId < 0) {

        Logger::warn(
          "MQTT SUB failed — cannot pull/push attributes"
        );
      }

      break;
    }

    // =====================================
    // SUBSCRIBED
    // =====================================

    case MQTT_EVENT_SUBSCRIBED: {

      Logger::info(
        "MQTT SUBSCRIBED msg_id=" +
        String(event->msg_id)
      );

      if (event->msg_id == subSharedId) {
        sharedSubAck = true;
      }
      if (event->msg_id == subResponseId) {
        responseSubAck = true;
      }
      if (
        sharedSubAck &&
        responseSubAck &&
        sharedPullDueAt == 0 &&
        sharedPullAt == 0 &&
        !sharedDataSeen
      ) {
        sharedPullDueAt = millis() + 400UL;
      }

      break;
    }

    // =====================================
    // DISCONNECTED
    // =====================================

    case MQTT_EVENT_DISCONNECTED: {

      const bool wasUp = mqttConnected;
      mqttConnected = false;
      sharedPullAt = 0;
      sharedPullDueAt = 0;
      sharedSubAck = false;
      responseSubAck = false;

      Logger::warn(
        "MQTT disconnected"
      );

      if (!failureArmed) {
        if (wasUp) {
          armReconnect(true, false);
        } else {
          armReconnect(true, true);
        }
      }

      break;
    }

    // =====================================
    // ERROR
    // =====================================

    case MQTT_EVENT_ERROR: {

      Logger::warn(
        "MQTT EVENT ERROR"
      );

      if (
        event->error_handle
      ) {

        const int tlsErr =
          event->error_handle
            ->esp_tls_last_esp_err;

        Logger::warn(
          "Error type: " +
          String(
            event->error_handle
            ->error_type
          )
        );

        Logger::warn(
          "ESP TLS error: " +
          String(tlsErr)
        );

        Logger::warn(
          "TLS stack error: " +
          String(
            event->error_handle
            ->esp_tls_stack_err
          )
        );

        Logger::warn(
          "Socket errno: " +
          String(
            event->error_handle
            ->esp_transport_sock_errno
          )
        );

      }

      if (
        event->error_handle &&
        (event->error_handle->connect_return_code == 4 ||
         event->error_handle->connect_return_code == 5)
      ) {
        Logger::warn(
          "MQTT broker refused the token (code " +
          String(event->error_handle->connect_return_code) +
          ")"
        );
        if (_refuseCb) {
          _refuseCb(event->error_handle->connect_return_code);
        }
      }

      if (!failureArmed) {
        armReconnect(true, true);
      }

      break;
    }

    // =====================================
    // DATA
    // =====================================

    case MQTT_EVENT_DATA: {

      String topicStr;
      String payload;

      for (
        int i = 0;
        i < event->topic_len;
        i++
      ) {

        topicStr +=
          event->topic[i];
      }

      for (
        int i = 0;
        i < event->data_len;
        i++
      ) {

        payload +=
          event->data[i];
      }

      Logger::info(
        "MQTT RX Topic: " +
        topicStr
      );

      Logger::info(
        "MQTT RX Payload: " +
        payload
      );

#if ARDUINOJSON_VERSION_MAJOR >= 7
      JsonDocument doc;
#else
      DynamicJsonDocument doc(8192);
#endif

      if (
        deserializeJson(
          doc,
          payload
        )
      ) {

        Logger::warn(
          "JSON parse failed"
        );

        return;
      }

      // ===================================
      // SHARED ATTRIBUTES
      // ===================================

      if (
        topicStr.indexOf(
          "/attributes/shared"
        ) >= 0
      ) {

        // Flat snapshot from platform. If a nested "shared" object exists, use it.
        JsonObject obj = doc.as<JsonObject>();
        if (
          !obj["shared"].isNull() &&
          obj["shared"].is<JsonObject>()
        ) {
          obj = obj["shared"].as<JsonObject>();
        }

        sharedDataSeen = true;
        sharedDataAt = millis();

        for (
          JsonPair kv : obj
        ) {

          String canonical;
          if (kv.value().is<const char*>() || kv.value().is<String>()) {
            const char* raw = kv.value().as<const char*>();
            canonical = raw ? raw : "";
          } else if (kv.value().is<bool>()) {
            canonical = kv.value().as<bool>() ? "1" : "0";
          } else {
            canonical = String(kv.value().as<float>(), 4);
          }
          if (duplicateShared(kv.key().c_str(), canonical)) {
            continue;
          }

          // Dashboard Attribute modal may store numbers as JSON strings ("3").
          // as<float>() on a string is often 0 — parse explicitly.
          if (kv.value().is<const char*>() || kv.value().is<String>()) {
            const char* s = kv.value().as<const char*>();
            if (!s) s = "";
            if (_attrStringCb) {
              _attrStringCb(kv.key().c_str(), String(s));
            }
            if (_attrCb) {
              char* end = nullptr;
              const float n = strtof(s, &end);
              if (end != s && end && *end == '\0') {
                _attrCb(kv.key().c_str(), n);
              }
            }
            continue;
          }

          if (kv.value().is<bool>()) {
            if (_attrCb) {
              _attrCb(
                kv.key().c_str(),
                kv.value().as<bool>() ? 1.0f : 0.0f
              );
            }
            continue;
          }

          if (_attrCb) {
            _attrCb(
              kv.key().c_str(),
              kv.value().as<float>()
            );
          }
        }
      }

      // ===================================
      // RPC
      // ===================================

      if (
        topicStr.indexOf(
          "/rpc/request/"
        ) >= 0
      ) {

        int idx =
          topicStr.lastIndexOf(
            "/"
          );

        if (idx >= 0) {

          lastRPCRequestId =
            topicStr.substring(
              idx + 1
            );
        }

        if (_rpcCb) {

          _rpcCb(
            doc["method"]
              .as<String>(),
            doc.as<JsonObject>()
          );
        }
      }

      break;
    }

    default:
      break;
  }
}

#endif