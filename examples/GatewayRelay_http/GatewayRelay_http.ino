// =============================================================
// GatewayRelay_http — gateway hub relays child telemetry (ESP32)
//
// Prerequisites:
//   1. Gateway device (Mark as Gateway Device) — use its token below.
//   2. Child device with parent set to that gateway — use its UUID below.
//
// POST /api/v1/{gatewayToken}/telemetry
// Body: { "childDeviceId": "<child-uuid>", "telemetry": { ... } }
//
// DEPENDENCIES: ArduinoJson, AutoconnectoSDK (for TLS roots only)
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

static const char* WIFI_SSID          = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD      = "YOUR_WIFI_PASSWORD";
static const char* API_HOST           = "api.autoconnecto.in";
static const char* GATEWAY_TOKEN      = "YOUR_GATEWAY_DEVICE_TOKEN";
static const char* CHILD_DEVICE_ID    = "YOUR_CHILD_DEVICE_UUID";

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
    if (time(nullptr) > 1700000000) return;
    delay(500);
  }
}

bool postTelemetry(const String& jsonBody, int* codeOut) {
  prepareTlsClient();
  HTTPClient http;
  String url = String("https://") + API_HOST + "/api/v1/" + GATEWAY_TOKEN + "/telemetry";
  if (!http.begin(tlsClient, url)) {
    *codeOut = -1;
    return false;
  }
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(25000);
  int c = http.POST(jsonBody);
  *codeOut = c;
  http.end();
  return c > 0;
}

unsigned long lastRelay = 0;

void setup() {
  Serial.begin(115200);
  delay(1500);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(400);

  syncTimeNtp();
  Serial.println("[HTTP] GatewayRelay_http ready");
}

void loop() {
  unsigned long now = millis();
  if (now - lastRelay < 15000) {
    delay(50);
    return;
  }
  lastRelay = now;

  StaticJsonDocument<384> doc;
  doc["childDeviceId"] = CHILD_DEVICE_ID;
  JsonObject tel = doc["telemetry"].to<JsonObject>();
  tel["temperature"] = (float)random(18, 32);
  tel["humidity"]    = (float)random(35, 75);

  String body;
  serializeJson(doc, body);

  int code = 0;
  if (postTelemetry(body, &code)) {
    Serial.printf("[RELAY] child=%s HTTP %d\n", CHILD_DEVICE_ID, code);
  }

  delay(50);
}
