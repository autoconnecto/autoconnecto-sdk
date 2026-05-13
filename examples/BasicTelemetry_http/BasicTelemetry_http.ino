// =============================================================
// BasicTelemetry_http — HTTPS device-token API (ESP32)
//
// Pair sketch: examples/BasicTelemetry_mqtt/BasicTelemetry_mqtt.ino
//
// PURPOSE
//   Same telemetry keys as BasicTelemetry_mqtt, without calling the
//   AutoconnectoSDK class (plain HTTPS). Install the AutoconnectoSDK
//   Arduino library anyway so `AutoconnectoIsrgRoots.h` resolves.
//   Uses POST /api/v1/:deviceToken/telemetry.
//
// DEPENDENCIES (Arduino Library Manager)
//   ArduinoJson
//   AutoconnectoSDK — required for `AutoconnectoIsrgRoots.h` (TLS roots).
//     This sketch does not call the SDK; install the library from this repo.
//
// TLS
//   Set USE_INSECURE_TLS_DEBUG to 1 only on a closed test bench.
// =============================================================

#ifndef USE_INSECURE_TLS_DEBUG
#define USE_INSECURE_TLS_DEBUG 0
#endif

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <AutoconnectoIsrgRoots.h>

// ---- Edit for your network and device --------------------------------

static const char* WIFI_SSID     = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
static const char* API_HOST      = "api.autoconnecto.in";
static const char* DEVICE_TOKEN  = "YOUR_DEVICE_TOKEN";

WiFiClientSecure tlsClient;

void prepareTlsClient() {
  tlsClient.stop();
#if USE_INSECURE_TLS_DEBUG
  tlsClient.setInsecure();
#else
  tlsClient.setCACert(AUTOCONNECTO_ROOT_CA);
#endif
}

void syncTimeNtp() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  for (int i = 0; i < 40; i++) {
    time_t t = time(nullptr);
    if (t > 1700000000) {
      Serial.println("[NTP] time synced");
      return;
    }
    delay(500);
  }
  Serial.println("[NTP] warning: time may be wrong — TLS can fail");
}

String apiPath(const char* suffix) {
  return String("/api/v1/") + DEVICE_TOKEN + suffix;
}

bool httpsExchange(const char* method, const String& path, const String& body, int* codeOut, String* respOut) {
  prepareTlsClient();
  HTTPClient http;
  String url = String("https://") + API_HOST + path;
  if (!http.begin(tlsClient, url)) {
    Serial.println("[HTTP] begin failed");
    *codeOut = -1;
    return false;
  }
  http.setTimeout(25000);
  int c = -1;
  if (strcmp(method, "GET") == 0) {
    c = http.GET();
  } else {
    http.addHeader("Content-Type", "application/json");
    c = http.POST(body.length() ? body : String("{}"));
  }
  *codeOut = c;
  if (c < 0) {
    Serial.printf("[HTTP] %s %s request error %d\n", method, path.c_str(), c);
    http.end();
    return false;
  }
  *respOut = http.getString();
  http.end();
  return true;
}

bool postTelemetryJson(const String& jsonBody, int* codeOut) {
  String resp;
  return httpsExchange("POST", apiPath("/telemetry"), jsonBody, codeOut, &resp);
}

unsigned long lastTelemetry   = 0;
unsigned long lastAttrReport  = 0;

void setup() {
  Serial.begin(115200);
  delay(1500);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi] connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.printf("\n[WiFi] IP %s\n", WiFi.localIP().toString().c_str());

  syncTimeNtp();

  int hc = 0;
  String hr;
  if (httpsExchange("GET", "/healthz", "", &hc, &hr)) {
    Serial.printf("[HEALTHZ] %d\n", hc);
  }

  Serial.println("[HTTP] BasicTelemetry_http started");
}

void loop() {
  unsigned long now = millis();

  if (now - lastTelemetry > 10000) {
    lastTelemetry = now;

    StaticJsonDocument<512> tel;
    tel["temperature"] = (float)random(20, 35);
    tel["humidity"]    = (float)random(40, 80);
    tel["current"]     = (float)random(1, 10);
    tel["power"]       = (float)random(100, 500);
    tel["voltage1"]    = (float)random(220, 280);
    tel["voltage2"]    = (float)random(220, 280);
    tel["voltage3"]    = (float)random(220, 280);

    String body;
    serializeJson(tel, body);
    int code = 0;
    if (postTelemetryJson(body, &code)) {
      Serial.printf("[TELEMETRY] %d\n", code);
    }
  }

  if (now - lastAttrReport > 30000) {
    lastAttrReport = now;
    StaticJsonDocument<256> attrs;
    attrs["freeHeap"]   = ESP.getFreeHeap();
    attrs["wifiRSSI"]   = WiFi.RSSI();
    attrs["uptime"]     = millis() / 1000;
    attrs["sdkVersion"] = 1.0;
    String body;
    serializeJson(attrs, body);
    int code = 0;
    String resp;
    if (httpsExchange("POST", apiPath("/attributes"), body, &code, &resp)) {
      Serial.printf("[CLIENT ATTR] %d\n", code);
    }
  }

  delay(50);
}
