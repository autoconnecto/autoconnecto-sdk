// =============================================================
// HttpControlPoll_http — HTTP-only desired-state control
//
// There is no server→device push over the device-token HTTPS API.
// HTTP devices must poll shared attributes, apply them, then
// POST client attributes so the dashboard can confirm.
//
//   GET  /api/v1/{token}/attributes/flat?scope=SHARED
//   POST /api/v1/{token}/attributes          (client scope default)
//
// For MQTT push of the same pattern see Attributes_ClientVsShared_mqtt
// or SwitchControl_mqtt. For a fuller HTTP switch panel see
// SwitchControl_http.
//
// Copy secrets.h.example → secrets.h before flashing.
// =============================================================

#ifndef USE_INSECURE_TLS_DEBUG
#define USE_INSECURE_TLS_DEBUG 0
#endif

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>
#include <AutoconnectoIsrgRoots.h>
#include "secrets.h"

static const char* API_HOST = "api.autoconnecto.in";
static const unsigned long POLL_MS = 2000;
static const unsigned long TEL_MS = 35000;

#define RELAY_PIN 2

WiFiClientSecure tls;
float lastRelayShared = NAN;
bool relayOn = false;

void prepareTls() {
  tls.stop();
#if USE_INSECURE_TLS_DEBUG
  tls.setInsecure();
#else
  tls.setCACert(AUTOCONNECTO_ROOT_CA);
#endif
}

String path(const char* suffix) {
  return String("/api/v1/") + DEVICE_TOKEN + suffix;
}

bool httpExchange(const char* method, const String& p, const String& body, int* code, String* resp) {
  prepareTls();
  HTTPClient http;
  if (!http.begin(tls, String("https://") + API_HOST + p)) {
    *code = -1;
    return false;
  }
  http.setTimeout(20000);
  int c = -1;
  if (strcmp(method, "GET") == 0) c = http.GET();
  else {
    http.addHeader("Content-Type", "application/json");
    c = http.POST(body.length() ? body : String("{}"));
  }
  *code = c;
  if (c < 0) {
    http.end();
    return false;
  }
  *resp = http.getString();
  http.end();
  return true;
}

void applyRelay(float raw) {
  const bool on = raw > 0.5f;
  if (on == relayOn && !isnan(lastRelayShared)) return;
  relayOn = on;
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);

  StaticJsonDocument<64> doc;
  doc["relay"] = on ? 1 : 0;
  String body;
  serializeJson(doc, body);
  int code = 0;
  String resp;
  httpExchange("POST", path("/attributes"), body, &code, &resp);
  Serial.printf("[APPLY] relay=%s client POST %d\n", on ? "ON" : "OFF", code);
}

void pollShared() {
  int code = 0;
  String resp;
  if (!httpExchange("GET", path("/attributes/flat?scope=SHARED"), "", &code, &resp)) {
    Serial.println("[POLL] GET failed");
    return;
  }
  if (code < 200 || code >= 300) {
    Serial.printf("[POLL] HTTP %d\n", code);
    return;
  }

  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, resp)) {
    Serial.println("[POLL] bad JSON");
    return;
  }

  // Flat map: { "relay": 1, "setpoint_c": 22, ... }
  if (!doc["relay"].isNull()) {
    const float v = doc["relay"].as<float>();
    if (isnan(lastRelayShared) || fabsf(lastRelayShared - v) > 1e-4f) {
      lastRelayShared = v;
      applyRelay(v);
    }
  }
  if (!doc["setpoint_c"].isNull()) {
    const float sp = doc["setpoint_c"].as<float>();
    StaticJsonDocument<64> conf;
    conf["applied_setpoint_c"] = sp;
    String body;
    serializeJson(conf, body);
    int c2 = 0;
    String r2;
    httpExchange("POST", path("/attributes"), body, &c2, &r2);
    Serial.printf("[POLL] setpoint_c=%.1f confirmed %d\n", sp, c2);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int i = 0; i < 60 && WiFi.status() != WL_CONNECTED; i++) delay(500);
  Serial.printf("[NET] %s\n", WiFi.status() == WL_CONNECTED ? "Wi-Fi ok" : "Wi-Fi failed");
  Serial.println("[SDK] HttpControlPoll_http — poll SHARED, confirm CLIENT");
}

unsigned long lastPoll = 0;
unsigned long lastTel = 0;

void loop() {
  if (WiFi.status() == WL_CONNECTED && millis() - lastPoll >= POLL_MS) {
    lastPoll = millis();
    pollShared();
  }
  if (WiFi.status() == WL_CONNECTED && millis() - lastTel >= TEL_MS) {
    lastTel = millis();
    StaticJsonDocument<96> tel;
    tel["relay"] = relayOn ? 1 : 0;
    tel["uptime_s"] = millis() / 1000UL;
    String body;
    serializeJson(tel, body);
    int code = 0;
    String resp;
    httpExchange("POST", path("/telemetry"), body, &code, &resp);
    Serial.printf("[TEL] %d\n", code);
  }
  delay(20);
}
