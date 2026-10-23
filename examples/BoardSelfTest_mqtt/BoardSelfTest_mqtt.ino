// =============================================================
// BoardSelfTest_mqtt — Autoconnecto SDK board verify sketch
//
// Covers attrs echo, RPC with replies, SOTA state reporting, and
// a forced reconnect path. Serial lines use clear [TAG] prefixes
// for support triage (BOARD_VERIFY.md B1–B7).
//
// Credentials: copy secrets.h.example → secrets.h (gitignored).
// =============================================================

#include <WiFi.h>
#include <AutoconnectoSDK.h>
#include <OtaUpdate.h>
#include <ArduinoJson.h>
#include "secrets.h"

AutoconnectoSDK sdk;
AutoconnectoOta ota;

#define RELAY_PIN_1 2
#define RELAY_PIN_2 4

bool ch[3] = {false, false, false};
unsigned long lastTel = 0;
unsigned long lastStat = 0;
unsigned long dropUntil = 0;
int reconnects = 0;
const char* OTA_KEYS =
  "sw_title,sw_version,sw_size,sw_checksum,sw_checksum_algorithm";

void applyChannel(int c, bool s) {
  if (c < 1 || c > 2) return;
  ch[c] = s;
  digitalWrite(c == 1 ? RELAY_PIN_1 : RELAY_PIN_2, s ? HIGH : LOW);
  sdk.sendClientAttribute("channel" + String(c), s ? 1.0f : 0.0f);
  Serial.printf("[ATTR] channel%d = %s\n", c, s ? "ON" : "OFF");
}

void onAttr(const String& k, float v) {
  Serial.printf("[ATTR] shared %s = %.2f\n", k.c_str(), v);
  ota.onSharedAttribute(k, v);
  if (k == "channel1") {
    applyChannel(1, v > 0.5f);
    return;
  }
  if (k == "channel2") {
    applyChannel(2, v > 0.5f);
    return;
  }
  if (k.startsWith("sw_")) return;
  sdk.sendClientAttribute(k, v);
  Serial.printf("[ATTR] client %s = %.2f\n", k.c_str(), v);
}

void onAttrS(const String& k, const String& v) {
  Serial.printf("[ATTR] shared(str) %s\n", k.c_str());
  ota.onSharedAttribute(k, v);
}

void onConn(bool) {
  Serial.printf("[NET] Connected reconnects=%d\n", reconnects);
  sdk.requestSharedAttributes();
  sdk.requestSharedAttributes(OTA_KEYS);
}

void onDisc(bool) {
  reconnects++;
  Serial.println("[NET] Disconnected");
}

void onRPC(const String& m, JsonObject p) {
  Serial.printf("[RPC] method=%s\n", m.c_str());
  JsonObject pr = p["params"].as<JsonObject>();
  JsonDocument r;
  if (m == "set_channel_state" || m == "relay_set") {
    int c = pr["channel"] | 1;
    bool s = (int)(pr["state"] | 0) > 0;
    if (c < 1 || c > 2) {
      r["success"] = false;
      r["message"] = "channel must be 1-2";
      sdk.replyRPC(r);
      return;
    }
    applyChannel(c, s);
    r["success"] = true;
    r["channel"] = c;
    r["state"] = s ? 1 : 0;
    Serial.printf("[RPC] reply %s ch=%d state=%d\n", m.c_str(), c, s ? 1 : 0);
    sdk.replyRPC(r);
    return;
  }
  if (m == "ping") {
    r["success"] = true;
    r["message"] = "pong";
    sdk.replyRPC(r);
    Serial.println("[RPC] reply pong");
    return;
  }
  if (m == "drop_network") {
    r["success"] = true;
    r["message"] = "dropping wifi 20s";
    sdk.replyRPC(r);
    dropUntil = millis() + 20000;
    return;
  }
  r["success"] = false;
  r["message"] = "Unknown method";
  sdk.replyRPC(r);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);

  SDKConfig config;
  config.wifiSSID = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;
  config.deviceToken = DEVICE_TOKEN;
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.mqttUseTls = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = false;

  OtaConfig oc;
  oc.apiHost = "api.autoconnecto.in";
  oc.deviceToken = DEVICE_TOKEN;
  oc.rootCA = AUTOCONNECTO_ROOT_CA;
  oc.autoReboot = false;
  ota.begin(oc, [](const char* k, const char* v) {
    JsonDocument d;
    d[k] = v;
    return sdk.sendClientAttributes(d);
  });
  ota.onProgressReport([](const char* sk, const char* st, const char* t, const char* v) {
    Serial.printf("[OTA-STATE] %s=%s\n", sk, st);
    JsonDocument d;
    d[sk] = st;
    d["sw_title"] = t;
    d["sw_version"] = v;
    return sdk.sendClientAttributes(d);
  });

  sdk.onAttributeUpdate(onAttr);
  sdk.onAttributeStringUpdate(onAttrS);
  sdk.onConnect(onConn);
  sdk.onDisconnect(onDisc);
  sdk.onRPC(onRPC);
  sdk.begin(config);
  Serial.println("[SDK] BoardSelfTest_mqtt started");
}

void loop() {
  if (dropUntil) {
    if ((long)(dropUntil - millis()) > 0) {
      if (WiFi.isConnected()) {
        Serial.println("[NET] test: WiFi.disconnect");
        WiFi.disconnect();
      }
      delay(50);
      return;
    }
    dropUntil = 0;
    Serial.println("[NET] test: WiFi.reconnect");
    WiFi.reconnect();
  }

  sdk.loop();
  bool wasBusy = ota.isBusy();
  ota.loop();
  if (wasBusy && !ota.isBusy()) sdk.requestSharedAttributes(OTA_KEYS);

  unsigned long now = millis();
  if (now - lastStat > 30000) {
    lastStat = now;
    Serial.printf(
      "[STATUS] connected=%d transport=%s heap=%u\n",
      sdk.connected(),
      sdk.activeTransport().c_str(),
      ESP.getFreeHeap()
    );
  }
  if (now - lastTel > 35000) {
    lastTel = now;
    JsonDocument t;
    t["board_temp"] = 20 + (now / 1000) % 10;
    t["board_uptime"] = now / 1000;
    bool ok = sdk.sendTelemetry(t);
    Serial.printf(
      "[TEL] sent ok=%d board_temp=%d board_uptime=%lu\n",
      ok,
      (int)t["board_temp"],
      now / 1000
    );
  }
  delay(10);
}
