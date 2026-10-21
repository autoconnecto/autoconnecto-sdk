// =============================================================
// WidgetCoverage_mqtt — Autoconnecto SDK
//
// PURPOSE
//   One board that exercises the telemetry / attribute / RPC keys
//   used by Phase 7 Solutions (widget palette coverage) and the
//   AllFunctionTest-style control widgets.
//
// PAIR WITH SOLUTIONS (exact titles in Solutions catalog)
//   MeteringChartsFleet, ControlsLabFleet, StatusWallboardFleet,
//   PlantScadaFleet, EnergyCommercialFleet, OpsClosedLoopFleet,
//   FacilityPlanFleet, SignalLabFleet — plus AllFunctionTest_mqtt.
//
// KEYS (telemetry unless noted)
//   Charts/gauges: temperature, humidity, active_power_kw, voltage_l1n,
//                  current_l1, level_pct, battery_pct, rssi_dbm
//   Plant:         motor_speed_rpm, valve_position_pct
//   Controls SHARED→CLIENT: channel1–4, volume, limitVoltage1–3
//   RPC:           ping, set_channel_state / relay_set
//
// Free plan: one telemetry JSON every 35 s.
// =============================================================

#include <AutoconnectoSDK.h>
#include <WiFi.h>

AutoconnectoSDK sdk;

#define RELAY_PIN_1 2
#define RELAY_PIN_2 4
#define RELAY_PIN_3 5
#define RELAY_PIN_4 18

bool channelState[5] = {false, false, false, false, false};
float volume = 0.0f;

void applyChannel(int ch, float raw) {
  if (ch < 1 || ch > 4) return;
  bool state = raw > 0.5f;
  channelState[ch] = state;
  int pin = -1;
  switch (ch) {
    case 1: pin = RELAY_PIN_1; break;
    case 2: pin = RELAY_PIN_2; break;
    case 3: pin = RELAY_PIN_3; break;
    case 4: pin = RELAY_PIN_4; break;
  }
  if (pin >= 0) digitalWrite(pin, state ? HIGH : LOW);
  sdk.sendClientAttribute("channel" + String(ch), state ? 1.0f : 0.0f);
}

void onAttributeUpdate(const String& key, float value) {
  Serial.printf("[ATTR] %s = %.2f\n", key.c_str(), value);
  if (key == "channel1") { applyChannel(1, value); return; }
  if (key == "channel2") { applyChannel(2, value); return; }
  if (key == "channel3") { applyChannel(3, value); return; }
  if (key == "channel4") { applyChannel(4, value); return; }
  if (key == "volume") {
    volume = value;
    sdk.sendClientAttribute("volume", volume);
  }
}

void onRPC(const String& method, JsonObject payload) {
  JsonObject params = payload["params"].as<JsonObject>();
  if (method == "ping") {
    StaticJsonDocument<64> reply;
    reply["success"] = true;
    reply["message"] = "pong";
    sdk.replyRPC(reply);
    return;
  }
  if (method == "set_channel_state" || method == "relay_set") {
    int ch = params["channel"] | 1;
    bool state = (int)(params["state"] | 0) > 0;
    applyChannel(ch, state ? 1.0f : 0.0f);
    StaticJsonDocument<96> reply;
    reply["success"] = true;
    reply["channel"] = ch;
    reply["state"] = state ? 1 : 0;
    sdk.replyRPC(reply);
    return;
  }
  StaticJsonDocument<96> unknown;
  unknown["success"] = false;
  unknown["method"] = method;
  sdk.replyRPC(unknown);
}

void onConnect(bool) {
  sdk.requestSharedAttributes();
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);

  SDKConfig config;
  config.wifiSSID = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort = 8084;
  config.deviceToken = "YOUR_DEVICE_TOKEN";
  config.enableWS = true;
  config.enableMQTT = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = true;

  sdk.onAttributeUpdate(onAttributeUpdate);
  sdk.onRPC(onRPC);
  sdk.onConnect(onConnect);
  sdk.begin(config);
  sdk.requestSharedAttributes();
  Serial.println("[SDK] WidgetCoverage_mqtt — pair with Phase 7 Solutions");
}

unsigned long lastTelemetry = 0;

void loop() {
  sdk.loop();
  unsigned long now = millis();
  if (now - lastTelemetry > 35000) {
    lastTelemetry = now;
    StaticJsonDocument<512> reading;
    reading["temperature"] = (float)random(18, 42);
    reading["humidity"] = (float)random(30, 90);
    reading["active_power_kw"] = (float)random(5, 100);
    reading["voltage_l1n"] = (float)random(210, 250);
    reading["current_l1"] = (float)random(1, 40);
    reading["level_pct"] = (float)random(10, 100);
    reading["battery_pct"] = (float)random(20, 100);
    reading["rssi_dbm"] = (float)WiFi.RSSI();
    reading["motor_speed_rpm"] = (float)random(0, 2800);
    reading["valve_position_pct"] = (float)random(0, 100);
    // AllFunctionTest-compatible aliases
    reading["current"] = reading["current_l1"];
    reading["power"] = reading["active_power_kw"].as<float>() * 1000.0f;
    reading["voltage1"] = reading["voltage_l1n"];
    reading["voltage2"] = reading["voltage_l1n"];
    reading["voltage3"] = reading["voltage_l1n"];
    sdk.sendTelemetry(reading);

    StaticJsonDocument<128> attrs;
    attrs["freeHeap"] = ESP.getFreeHeap();
    attrs["wifiRSSI"] = WiFi.RSSI();
    attrs["uptime"] = millis() / 1000;
    attrs["sdkVersion"] = 1.0;
    sdk.sendClientAttributes(attrs);
  }
  delay(10);
}
