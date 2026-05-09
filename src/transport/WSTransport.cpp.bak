#include "WSTransport.h"

WSTransport* WSTransport::instance = nullptr;

WSTransport::WSTransport() {
  instance = this;
}

void WSTransport::configure(
  SDKConfig* config,
  AttributeCallback attrCb
) {

  _config = config;
  _attrCb = attrCb;
}

bool WSTransport::begin() {

  Logger::info("Initializing WS transport");

  Logger::info(
    String("WS HOST=") + _config->wsHost
  );

  Logger::info(
    String("WS PORT=") + String(_config->wsPort)
  );

  Logger::info(
    String("WS PATH=") + _config->wsPath
  );

  delay(2000);

  ws.begin(
    _config->wsHost.c_str(),
    _config->wsPort,
    _config->wsPath.c_str()
  );

  ws.onEvent(wsEventStatic);

  ws.setReconnectInterval(5000);
  ws.enableHeartbeat(
  15000,
  3000,
  2
);

  Logger::info("WS begin issued");

  return true;
}

void WSTransport::loop() {
  ws.loop();
}

bool WSTransport::connected() {
  return wsConnected && wsAuthed;
}

bool WSTransport::sendTelemetry(
  const String& payload
) {

  StaticJsonDocument<512> root;
  StaticJsonDocument<256> data;

  DeserializationError err =
    deserializeJson(data, payload);

  if (err) {

    Logger::error(
      String("WS telemetry JSON parse failed: ") +
      err.c_str()
    );

    return false;
  }

  root["type"] = "telemetry";

  root["data"] = data.as<JsonObject>();

  String out;

  serializeJson(root, out);

  Logger::info(
    String("WS TX: ") + out
  );

  ws.sendTXT(out);

  return true;
}

bool WSTransport::sendClientAttribute(
  const String& key,
  float value
) {

  StaticJsonDocument<128> doc;

  doc["type"] = "client_attribute";
  doc["key"] = key;
  doc["value"] = value;

  String out;

  serializeJson(doc, out);

  ws.sendTXT(out);

  return true;
}

void WSTransport::requestAttributes() {

  StaticJsonDocument<256> doc;

  doc["type"] = "get_attributes";

  doc["scope"] = "SHARED";

  String out;

  serializeJson(doc, out);

  ws.sendTXT(out);
}

void WSTransport::wsEventStatic(
  WStype_t type,
  uint8_t* payload,
  size_t length
) {

  if (instance) {
    instance->wsEvent(
      type,
      payload,
      length
    );
  }
}

void WSTransport::wsEvent(
  WStype_t type,
  uint8_t* payload,
  size_t length
) {

  switch(type) {

    case WStype_CONNECTED: {

      Logger::info("WS connected");
      Logger::info("WS sending auth");
      wsConnected = true;
      wsAuthed = false;

      StaticJsonDocument<128> doc;

      doc["type"] = "auth";
      doc["deviceToken"] = _config->deviceToken;

      String out;

      serializeJson(doc, out);

      ws.sendTXT(out);

      break;
    }

    case WStype_DISCONNECTED: {

      Logger::warn("WS disconnected");

      wsConnected = false;
      wsAuthed = false;

      break;
    }

    case WStype_TEXT: {
      Logger::info(
        String("WS RX: ") +
        String((char*)payload)
      );
      StaticJsonDocument<1024> doc;

      if (deserializeJson(doc, payload)) {
        
        return;
      }

      String type = doc["type"] | "";

      if (type == "auth_result") {

        wsAuthed = doc["success"] | false;

        Logger::info(
          String("WS auth=") +
          (wsAuthed ? "OK" : "FAIL")
        );

        if (wsAuthed) {
          requestAttributes();
        }

        return;
      }

      if (type == "attributes_response") {

        JsonObject data = doc["data"];

        for (JsonPair kv : data) {

          if (_attrCb) {

            _attrCb(
              kv.key().c_str(),
              kv.value().as<float>()
            );
          }
        }

        return;
      }

      if (type == "attribute_update") {

        if (_attrCb) {

          _attrCb(
            doc["key"] | "",
            doc["value"] | 0
          );
        }

        return;
      }
    }
  }
}