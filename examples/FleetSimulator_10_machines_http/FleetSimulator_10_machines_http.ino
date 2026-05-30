// =============================================================
// FleetSimulator_10_machines_http — simulate 10 devices from ONE ESP32
//
// PURPOSE
//   Rotate HTTPS telemetry (and optional client attributes) across
//   10 device tokens so you can test dashboards, data pipelines,
//   machine fleet widgets, alarms, and realtime without 10 boards.
//
// SETUP
//   1. Paste each device's TOKEN below (Devices → open device → copy token).
//   2. Set WIFI_* and API_HOST (production or local).
//   3. Install: ArduinoJson + AutoconnectoSDK (for TLS roots).
//
// LOCAL (app.local.autoconnecto):
//   #define API_HOST "app.local.autoconnecto"
//   #define USE_INSECURE_TLS_DEBUG 1
//
// GET TOKENS (Postgres on dev/prod):
//   SELECT device_id, device_name, device_token
//   FROM devices
//   WHERE device_id IN (
//     '88d6da44-3a99-4373-94c7-9d1d1aa40be7',
//     ...
//   );
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
#include <math.h>

// ---- Network & API -------------------------------------------------------

static const char* WIFI_SSID     = "71";
static const char* WIFI_PASSWORD = "90946062";

// Production:
//static const char* API_HOST = "api.autoconnecto.in";
// Local Caddy (uncomment):
static const char* API_HOST = "192.168.68.107";

static const unsigned long TELEMETRY_INTERVAL_MS = 12000;  // per device slot
static const unsigned long FULL_CYCLE_PAUSE_MS   = 2000;

// ---- Fleet: device id (reference) + token (required) ---------------------

struct FleetDevice {
  const char* deviceId;   // for Serial logs only
  const char* label;
  const char* token;      // paste from platform UI
};

// Tokens: replace REPLACE_WITH_TOKEN_N from Devices → device details.
static FleetDevice kFleet[] = {
  { "88d6da44-3a99-4373-94c7-9d1d1aa40be7", "machine-01", "REPLACE_WITH_TOKEN_1" },
  { "ff13ef77-56f0-4805-b865-2495ffd49734", "machine-02", "REPLACE_WITH_TOKEN_2" },
  { "88b88a74-4af8-4424-9461-c97c2ce7088a", "machine-03", "REPLACE_WITH_TOKEN_3" },
  { "e4f9e53f-f312-46e0-91bd-63db4644c05a", "machine-04", "REPLACE_WITH_TOKEN_4" },
  { "2e4a59b6-535f-4cfe-ac73-5706a66b3038", "machine-05", "REPLACE_WITH_TOKEN_5" },
  { "afb363fb-0b29-421e-b291-8c06aca19ece", "machine-06", "REPLACE_WITH_TOKEN_6" },
  { "72c6a849-2ce5-4760-94a5-e1bfb673d74a", "machine-07", "REPLACE_WITH_TOKEN_7" },
  { "950848a4-d873-4779-b433-b6d99605fc53", "machine-08", "REPLACE_WITH_TOKEN_8" },
  { "5ea53525-9868-4be3-bbd8-b9f2f1868395", "machine-09", "REPLACE_WITH_TOKEN_9" },
  { "840415bf-a261-45b1-a76a-4fd29d896266", "machine-10", "REPLACE_WITH_TOKEN_10" },
};

static const size_t kFleetCount = sizeof(kFleet) / sizeof(kFleet[0]);

WiFiClientSecure tlsClient;
size_t fleetIndex = 0;
unsigned long lastSendMs = 0;

// Per-device phase so charts don't look identical
static float simulatedMachineAmps(size_t idx, unsigned long t) {
  const float base = 2.0f + (float)(idx % 5) * 1.5f;
  const float wave = sinf((t / 1000.0f) * 0.05f + (float)idx) * 2.5f;
  return base + wave + (float)random(0, 80) / 100.0f;
}

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
    if (time(nullptr) > 1700000000) {
      Serial.println("[NTP] OK");
      return;
    }
    delay(500);
  }
  Serial.println("[NTP] warn: TLS may fail without valid time");
}

bool httpsPost(const char* token, const char* pathSuffix, const String& jsonBody, int* codeOut) {
  prepareTlsClient();
  HTTPClient http;
  String url = String("https://") + API_HOST + "/api/v1/" + token + pathSuffix;

  if (!http.begin(tlsClient, url)) {
    *codeOut = -1;
    return false;
  }
  http.setTimeout(25000);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(jsonBody.length() ? jsonBody : String("{}"));
  *codeOut = code;
  if (code > 0) {
    String resp = http.getString();
    if (code < 200 || code >= 300) {
      Serial.printf("[HTTP] %s → %d %s\n", pathSuffix, code, resp.c_str());
    }
  } else {
    Serial.printf("[HTTP] POST failed err=%d url=%s\n", code, url.c_str());
  }
  http.end();
  return code >= 200 && code < 300;
}

void buildTelemetryPayload(size_t idx, unsigned long nowMs, JsonDocument& doc) {
  doc.clear();
  const float amps = simulatedMachineAmps(idx, nowMs);

  // Machine Fleet widget + general charts
  doc["machine_current_a"] = amps;
  doc["machine_running"]   = amps > 3.0f;
  doc["machine_id_label"]  = kFleet[idx].label;

  // Standard dashboard keys (AllFunctionTest / BasicTelemetry)
  doc["temperature"] = 20.0f + (float)(idx * 2) + (float)random(0, 50) / 10.0f;
  doc["humidity"]    = 45.0f + (float)random(0, 200) / 10.0f;
  doc["current"]     = amps;
  doc["power"]       = amps * 230.0f;
  doc["voltage1"]    = 228.0f + (float)random(0, 80) / 10.0f;
  doc["voltage2"]    = 229.0f + (float)random(0, 80) / 10.0f;
  doc["voltage3"]    = 227.0f + (float)random(0, 80) / 10.0f;

  doc["fleet_index"]   = (int)idx + 1;
  doc["simulator"]     = true;
  doc["ts"]            = (long long)nowMs;
}

void buildClientAttributes(size_t idx, JsonDocument& doc) {
  doc.clear();
  doc["freeHeap"]      = ESP.getFreeHeap();
  doc["wifiRSSI"]      = WiFi.RSSI();
  doc["uptime"]        = millis() / 1000;
  doc["fleet_label"]   = kFleet[idx].label;
  doc["fleet_deviceId"] = kFleet[idx].deviceId;
  doc["sdkVersion"]    = "fleet-sim-1.0";
}

bool tokenConfigured(const char* token) {
  if (!token || !token[0]) return false;
  if (strncmp(token, "REPLACE_WITH_TOKEN", 18) == 0) return false;
  return true;
}

void sendForDevice(size_t idx) {
  const FleetDevice& dev = kFleet[idx];
  if (!tokenConfigured(dev.token)) {
    Serial.printf("[SKIP] %s — paste token for %s\n", dev.label, dev.deviceId);
    return;
  }

  StaticJsonDocument<768> tel;
  buildTelemetryPayload(idx, millis(), tel);
  String telBody;
  serializeJson(tel, telBody);

  int code = 0;
  if (httpsPost(dev.token, "/telemetry", telBody, &code)) {
    Serial.printf("[OK] %s (%s) telemetry HTTP %d amps=%.2f\n",
                  dev.label, dev.deviceId, code,
                  tel["machine_current_a"].as<float>());
  }

  StaticJsonDocument<384> attrs;
  buildClientAttributes(idx, attrs);
  String attrBody;
  serializeJson(attrs, attrBody);
  if (httpsPost(dev.token, "/attributes", attrBody, &code)) {
    Serial.printf("[OK] %s client attrs HTTP %d\n", dev.label, code);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  randomSeed(esp_random());

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[WiFi]");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.printf(" IP %s\n", WiFi.localIP().toString().c_str());

  syncTimeNtp();

  Serial.printf("[FLEET] %u devices, interval %lu ms, host %s\n",
                (unsigned)kFleetCount, TELEMETRY_INTERVAL_MS, API_HOST);
  Serial.println("[FLEET] Paste tokens in kFleet[] then re-upload.");
}

void loop() {
  const unsigned long now = millis();
  if (now - lastSendMs < TELEMETRY_INTERVAL_MS) {
    delay(50);
    return;
  }
  lastSendMs = now;

  sendForDevice(fleetIndex);
  fleetIndex = (fleetIndex + 1) % kFleetCount;

  if (fleetIndex == 0) {
    Serial.println("[FLEET] --- full cycle done, pausing ---");
    delay(FULL_CYCLE_PAUSE_MS);
  }
}
