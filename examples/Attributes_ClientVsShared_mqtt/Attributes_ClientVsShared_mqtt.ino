// =============================================================
// Attributes_ClientVsShared_mqtt — two scopes, one sketch
//
// SHARED (platform → device)
//   Written from the dashboard / API. Pushed live on MQTT and
//   restored after boot via requestSharedAttributes().
//   Example keys: setpoint_c, enable_heater
//
// CLIENT (device → platform)
//   Reported by the device. Dashboards read these to confirm
//   hardware state (serial, firmware, applied setpoint).
//   Example keys: serial, firmware, applied_setpoint_c, heater_on
//
// Pattern:
//   1) Shared arrives → apply to hardware
//   2) Send matching client attrs so the UI can show confirmation
//   3) On connect, request shared attrs to rebuild after power loss
//
// Copy secrets.h.example → secrets.h before flashing.
// =============================================================

#include <AutoconnectoSDK.h>
#include "secrets.h"

AutoconnectoSDK sdk;

float setpointC = 22.0f;
bool heaterOn = false;
const char* SERIAL_NO = "AC-DEMO-001";
const char* FIRMWARE = "attrs-demo-1.0.0";

void reportIdentity() {
  JsonDocument d;
  d["serial"] = SERIAL_NO;
  d["firmware"] = FIRMWARE;
  d["applied_setpoint_c"] = setpointC;
  d["heater_on"] = heaterOn ? 1 : 0;
  sdk.sendClientAttributes(d);
  Serial.println("[CLIENT] identity + applied state reported");
}

void applyShared(const String& key, float value) {
  Serial.printf("[SHARED] %s = %.2f\n", key.c_str(), value);
  if (key == "setpoint_c") {
    setpointC = value;
    sdk.sendClientAttribute("applied_setpoint_c", setpointC);
    return;
  }
  if (key == "enable_heater") {
    heaterOn = value > 0.5f;
    // Drive a real GPIO here in production.
    sdk.sendClientAttribute("heater_on", heaterOn ? 1.0f : 0.0f);
    return;
  }
}

void onAttr(const String& key, float value) { applyShared(key, value); }

void onAttrS(const String& key, const String& value) {
  Serial.printf("[SHARED str] %s = %s\n", key.c_str(), value.c_str());
  // String shared attrs (e.g. mode) — echo as client for visibility.
  JsonDocument d;
  d[key] = value;
  sdk.sendClientAttributes(d);
}

void onConn(bool) {
  Serial.println("[NET] connected — request shared attrs");
  reportIdentity();
  sdk.requestSharedAttributes();
  sdk.requestSharedAttributes("setpoint_c,enable_heater");
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  SDKConfig config;
  config.wifiSSID = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;
  config.deviceToken = DEVICE_TOKEN;
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort = 8084;
  config.enableWS = true;
  config.enableMQTT = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = false;

  sdk.onAttributeUpdate(onAttr);
  sdk.onAttributeStringUpdate(onAttrS);
  sdk.onConnect(onConn);
  sdk.begin(config);
  Serial.println("[SDK] Attributes_ClientVsShared_mqtt started");
}

unsigned long lastBeat = 0;

void loop() {
  sdk.loop();
  // Light telemetry so the device stays Active while you poke shared attrs.
  if (millis() - lastBeat > 35000UL) {
    lastBeat = millis();
    JsonDocument tel;
    tel["room_temp_c"] = setpointC + (heaterOn ? 0.4f : -0.2f);
    sdk.sendTelemetry(tel);
  }
  delay(10);
}
