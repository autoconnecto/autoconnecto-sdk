// =============================================================
// TelemetryBatch_http — HTTPS batch ingest (ESP32)
//
// Sends multiple telemetry samples in one POST:
//   POST /api/v1/{deviceToken}/telemetry/batch
//
// Pair with BasicTelemetry_http for single-sample uploads.
// Max 100 items per request (platform limit).
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
    if (time(nullptr) > 1700000000) return;
    delay(500);
  }
}

bool postBatch(const String& jsonBody, int* codeOut) {
  prepareTlsClient();
  HTTPClient http;
  String url = String("https://") + API_HOST + "/api/v1/" + DEVICE_TOKEN + "/telemetry/batch";
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

unsigned long lastBatch = 0;

void setup() {
  Serial.begin(115200);
  delay(1500);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(400);

  syncTimeNtp();
  Serial.println("[HTTP] TelemetryBatch_http ready");
}

void loop() {
  unsigned long now = millis();
  if (now - lastBatch < 30000) {
    delay(50);
    return;
  }
  lastBatch = now;

  StaticJsonDocument<768> doc;
  JsonArray items = doc["items"].to<JsonArray>();

  for (int i = 0; i < 3; i++) {
    JsonObject item = items.add<JsonObject>();
    JsonObject tel = item["telemetry"].to<JsonObject>();
    tel["temperature"] = (float)random(20, 35);
    tel["humidity"]    = (float)random(40, 80);
    item["ts"] = (int64_t)(time(nullptr) * 1000LL) + (i * 1000);
  }

  String body;
  serializeJson(doc, body);

  int code = 0;
  if (postBatch(body, &code)) {
    Serial.printf("[BATCH] HTTP %d (%u bytes)\n", code, body.length());
  } else {
    Serial.println("[BATCH] request failed");
  }

  delay(50);
}
