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
  AttributeCallback attrCb
) {

  _config = config;

  _attrCb = attrCb;
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
    !mqttConnected
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

  // LTE PPP: MQTTS only (cellular). WiFi: unchanged WSS primary path.
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

  usingWSS = true;

  return
    "wss://" +
    _config->mqttHost +
    ":" +
    String(_config->wssPort) +
    "/mqtt";
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
  }

  doc["requestId"] =
    millis();

  String out;

  serializeJson(doc, out);

  publish(
    topic("attributes/shared/request"),
    out
  );
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

      Logger::info(
        usingWSS
        ? "Connected via WSS"
        : "Connected via MQTTS"
      );

      // ===================================
      // SUBSCRIPTIONS
      // ===================================

      esp_mqtt_client_subscribe(
        client,
        topic(
          "attributes/shared"
        ).c_str(),
        1
      );

      esp_mqtt_client_subscribe(
        client,
        topic(
          "attributes/shared/response"
        ).c_str(),
        1
      );

      esp_mqtt_client_subscribe(
        client,
        topic(
          "rpc/request/+"
        ).c_str(),
        1
      );

      // ===================================
      // INITIAL ATTRIBUTE FETCH
      // ===================================

      requestAttributes();

      break;
    }

    // =====================================
    // DISCONNECTED
    // =====================================

    case MQTT_EVENT_DISCONNECTED: {

      mqttConnected = false;

      Logger::warn(
        "MQTT disconnected"
      );

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

        Logger::warn(
          "Error type: " +
          String(
            event->error_handle
            ->error_type
          )
        );

        Logger::warn(
          "ESP TLS error: " +
          String(
            event->error_handle
            ->esp_tls_last_esp_err
          )
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