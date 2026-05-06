#include "AutoconnectoSDK.h"
#include "core/Config.h"
void AutoconnectoSDK::begin(
  SDKConfig config
) {

  _config = config;

  Logger::begin(
    _config.enableSerialLogs
  );

  connectWiFi();

  manager.configure(
    &_config,
    attrCb
  );

  manager.begin();
}

void AutoconnectoSDK::loop() {

  manager.loop();
}

void AutoconnectoSDK::onAttributeUpdate(
  AttributeCallback cb
) {

  attrCb = cb;
}

bool AutoconnectoSDK::connected() {

  return manager.activeTransport() != nullptr;
}

bool AutoconnectoSDK::sendTelemetryJson(
  const String& json
) {

  ITransport* t =
    manager.activeTransport();

  if (!t) {
    return false;
  }

  return t->sendTelemetry(json);
}

bool AutoconnectoSDK::sendTelemetry(
  JsonDocument& doc
) {

  String out;

  serializeJson(doc, out);

  return sendTelemetryJson(out);
}

bool AutoconnectoSDK::sendClientAttribute(
  const String& key,
  float value
) {

  ITransport* t =
    manager.activeTransport();

  if (!t) {
    return false;
  }

  return t->sendClientAttribute(
    key,
    value
  );
}

void AutoconnectoSDK::connectWiFi() {

  Logger::info("Connecting WiFi");

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

  Logger::info("WiFi connected");
}