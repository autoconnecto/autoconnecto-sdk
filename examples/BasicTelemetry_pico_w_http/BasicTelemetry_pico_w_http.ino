// =============================================================
// BasicTelemetry_pico_w_http — HTTPS on Raspberry Pi Pico W
//
// Core: Raspberry Pi Pico/RP2040 by Earle Philhower.
// Board: Raspberry Pi Pico W.
//
// HTTP first. This board does not use the ESP32 MQTT client.
// Install AutoconnectoSDK so AutoconnectoIsrgRoots.h resolves.
// TLS is on.
//
// A reading the plan refuses comes back as HTTP 429. The body
// is printed. Free stores one reading every 30 seconds.
// =============================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <AutoconnectoIsrgRoots.h>

static const char* WIFI_SSID = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
static const char* API_HOST = "api.autoconnecto.in";
static const char* DEVICE_TOKEN = "YOUR_DEVICE_TOKEN";

WiFiClientSecure tlsClient;

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
  tlsClient.setCACert(AUTOCONNECTO_ROOT_CA);
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

unsigned long lastTelemetry = 0;
unsigned long lastAttr = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi] connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.printf("\n[WiFi] IP %s\n", WiFi.localIP().toString().c_str());
  syncTimeNtp();
  Serial.println("[HTTP] BasicTelemetry_pico_w_http started");
}

void loop() {
  unsigned long now = millis();

  if (now - lastTelemetry > 30000) {
    lastTelemetry = now;
    JsonDocument tel;
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
    JsonDocument attrs;
    attrs["uptime"] = millis() / 1000;
    String body;
    serializeJson(attrs, body);
    int code = 0;
    String resp;
    if (httpsExchange("POST", apiPath("/attributes"), body, &code, &resp)) {
      printRefusal("CLIENT ATTR", code, resp);
    }
  }

  delay(50);
}
