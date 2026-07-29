// =========================================
// MQTTTransport.cpp
// =========================================

#include "MQTTTransport.h"

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

void MQTTTransport::loop() {

  if (
    reconnectRequested &&
    !mqttConnected &&
    millis() >= reconnectEarliestMs
  ) {

    reconnectRequested = false;

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
  mqtt_cfg.network.reconnect_timeout_ms = 10000;

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

      subscribePending = 0;

      if (
        esp_mqtt_client_subscribe(
          client,
          sharedTopic.c_str(),
          1
        ) >= 0
      ) {

        subscribePending++;
      }

      if (
        esp_mqtt_client_subscribe(
          client,
          responseTopic.c_str(),
          1
        ) >= 0
      ) {

        subscribePending++;
      }

      if (
        esp_mqtt_client_subscribe(
          client,
          rpcTopic.c_str(),
          1
        ) >= 0
      ) {

        subscribePending++;
      }

      if (subscribePending == 0) {

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

      if (subscribePending > 0) {

        subscribePending--;

        if (subscribePending == 0) {

          const String keys =
            _config
              ? _config->sharedAttributeKeys
              : "";

          Logger::info(
            "MQTT pull shared attributes" +
            (keys.length()
              ? " keys=" + keys
              : " (all)")
          );

          requestAttributes(keys);
        }
      }

      break;
    }

    // =====================================
    // DISCONNECTED
    // =====================================

    case MQTT_EVENT_DISCONNECTED: {

      mqttConnected = false;
      subscribePending = 0;

      Logger::warn(
        "MQTT disconnected"
      );

      reconnectEarliestMs =
        millis() + 3000;

      reconnectRequested =
        true;

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

        // 32774 = ESP_ERR_ESP_TLS_CONNECTION_TIMEOUT — try other WiFi port
        if (
          tlsErr == 32774 &&
          _config &&
          _config->networkMode ==
            NetworkMode::WiFi
        ) {

          wifiUseAlternateTransport =
            !wifiUseAlternateTransport;

          Logger::warn(
            wifiUseAlternateTransport
              ? "MQTT timeout — trying alternate transport"
              : "MQTT timeout — trying primary transport"
          );
        }
      }

      reconnectEarliestMs =
        millis() + 3000;

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

      StaticJsonDocument<512> doc;

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
        topicStr.endsWith(
          "/attributes/shared"
        ) ||

        topicStr.endsWith(
          "/attributes/shared/response"
        )
      ) {

        JsonObject obj =
          doc.as<JsonObject>();

        for (
          JsonPair kv : obj
        ) {

          // Dashboard Attribute modal may store numbers as JSON strings ("3").
          // as<float>() on a string is often 0 — parse explicitly.
          if (kv.value().is<const char*>() || kv.value().is<String>()) {
            const char* s = kv.value().as<const char*>();
            if (_attrStringCb) {
              _attrStringCb(kv.key().c_str(), String(s ? s : ""));
            }
            if (_attrCb) {
              _attrCb(kv.key().c_str(), (float)atof(s ? s : "0"));
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