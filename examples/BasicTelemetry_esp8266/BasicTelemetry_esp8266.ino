// =============================================================
// BasicTelemetry_esp8266 — HTTPS on ESP8266
//
// ESP8266 does not have the RAM for MQTT-over-WSS. This sketch
// uses HTTPS only: telemetry, client attributes, and shared
// attributes (the command path on HTTP).
//
// Board: ESP8266 core. Install AutoconnectoSDK for
// AutoconnectoIsrgRoots.h. TLS is on. Do not set insecure
// unless the bench has no clock.
//
// A reading the plan refuses comes back as HTTP 429. The body
// is printed. Free stores one reading every 30 seconds.
// =============================================================

#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <AutoconnectoIsrgRoots.h>
#include "secrets.h"
static const char* API_HOST = "api.autoconnecto.in";
static const int COMMAND_PIN = 2;

BearSSL::WiFiClientSecure tlsClient;
BearSSL::X509List trustAnchors(AUTOCONNECTO_ROOT_CA);

String apiPath(const char* suffix) {
  return String("/api/v1/") + DEVICE_TOKEN + suffix;
}

bool httpsExchange(
  const char* method,
  const String& path,
  const String& body,
  int* codeOut,
  String* respOut
) {
  tlsClient.setTrustAnchors(&trustAnchors);
  tlsClient.setBufferSizes(1024, 1024);

  HTTPClient http;
  String url = String("https://") + API_HOST + path;
  if (!http.begin(tlsClient, url)) {
    *codeOut = -1;
    return false;
  }
  http.setTimeout(20000);
  int code = -1;
  if (strcmp(method, "GET") == 0) {
    code = http.GET();
  } else {
    http.addHeader("Content-Type", "application/json");
    code = http.POST(body.length() ? body : String("{}"));
  }
  *codeOut = code;
  if (code > 0) *respOut = http.getString();
  http.end();
  return code > 0;
}

void printRefusal(const char* what, int code, const String& body) {
  if (code == 200 || code == 201) {
    Serial.printf("[%s] %d\n", what, code);
    return;
  }
  Serial.printf("[%s] refused %d %s\n", what, code, body.c_str());
}

void syncTimeNtp() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  for (int i = 0; i < 40; i++) {
    if (time(nullptr) > 1700000000) {
      Serial.println("[NTP] time synced");
      return;
    }
    delay(500);
  }
  Serial.println("[NTP] warning: time may be wrong — TLS can fail");
}

void pollShared() {
  int code = 0;
  String resp;
  if (!httpsExchange("GET", apiPath("/attributes/flat") + "?scope=SHARED", "", &code, &resp)) {
    Serial.printf("[SHARED] request failed %d\n", code);
    return;
  }
  if (code != 200) {
    printRefusal("SHARED", code, resp);
    return;
  }

  DynamicJsonDocument doc(2048);
  if (deserializeJson(doc, resp)) {
    Serial.println("[SHARED] JSON parse error");
    return;
  }
  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject item : arr) {
    const char* key = item["key"] | "";
    if (!key[0]) continue;
    JsonVariant value = item["value"];
    Serial.printf("[SHARED] %s=", key);
    if (value.is<const char*>()) {
      Serial.println(value.as<const char*>());
    } else {
      Serial.println(value.as<float>(), 3);
    }
    if (strcmp(key, "relay") == 0) {
      digitalWrite(COMMAND_PIN, value.as<float>() > 0.5f ? HIGH : LOW);
    }
  }
}

unsigned long lastTelemetry = 0;
unsigned long lastAttr = 0;
unsigned long lastShared = 0;

void setup() {
  Serial.begin(115200);
  delay(1500);
  pinMode(COMMAND_PIN, OUTPUT);
  digitalWrite(COMMAND_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi] connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.printf("\n[WiFi] IP %s\n", WiFi.localIP().toString().c_str());
  syncTimeNtp();
  Serial.println("[HTTP] BasicTelemetry_esp8266 started");
}

void loop() {
  unsigned long now = millis();

  if (now - lastTelemetry > 30000) {
    lastTelemetry = now;
    StaticJsonDocument<128> tel;
    tel["temperature"] = (float)random(20, 35);
    tel["humidity"] = (float)random(40, 80);
    String body;
    serializeJson(tel, body);
    int code = 0;
    String resp;
    if (httpsExchange("POST", apiPath("/telemetry"), body, &code, &resp)) {
      printRefusal("TELEMETRY", code, resp);
    }
  }

  if (now - lastAttr > 60000) {
    lastAttr = now;
    StaticJsonDocument<128> attrs;
    attrs["freeHeap"] = ESP.getFreeHeap();
    attrs["wifiRSSI"] = WiFi.RSSI();
    attrs["uptime"] = millis() / 1000;
    String body;
    serializeJson(attrs, body);
    int code = 0;
    String resp;
    if (httpsExchange("POST", apiPath("/attributes"), body, &code, &resp)) {
      printRefusal("CLIENT ATTR", code, resp);
    }
  }

  if (now - lastShared > 15000) {
    lastShared = now;
    pollShared();
  }

  delay(50);
}
