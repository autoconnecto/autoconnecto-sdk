// =============================================================
// AllFunctionTest_http — HTTPS device-token API (ESP32)
//
// Pair sketch: examples/AllFunctionTest_mqtt/AllFunctionTest_mqtt.ino
//
// PURPOSE
//   Same telemetry keys, shared-attribute keys, relay wiring, and
//   client-attribute confirmations as AllFunctionTest_mqtt.
//   Shared attributes are polled over HTTPS (no server push).
//
// RPC (dashboard widget)
//   Not available on device-token HTTPS. Methods implemented on
//   the MQTT sketch (ping, getStatus, getConfig, getDiagnostics,
//   setValue, relay_set, reset, reboot, telemetry_burst) require
//   AllFunctionTest_mqtt or a server-side JWT command API.
//
// DEPENDENCIES
//   ArduinoJson
//   AutoconnectoSDK library — for `AutoconnectoIsrgRoots.h` (TLS roots) only.
//
// TLS: set USE_INSECURE_TLS_DEBUG to 1 only on a test bench.
// =============================================================

#ifndef USE_INSECURE_TLS_DEBUG
#define USE_INSECURE_TLS_DEBUG 0
#endif

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <math.h>
#include <AutoconnectoIsrgRoots.h>

static const char* WIFI_SSID     = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
static const char* API_HOST      = "api.autoconnecto.in";
static const char* DEVICE_TOKEN  = "YOUR_DEVICE_TOKEN";

static const unsigned long POLL_SHARED_MS = 750;

#define RELAY_PIN_1   2
#define RELAY_PIN_2   4
#define RELAY_PIN_3   5
#define RELAY_PIN_4   18

bool  channelState[5] = {false, false, false, false, false};
float volume        = 0.0f;
float limitVoltage1 = 0.0f;
float limitVoltage2 = 0.0f;
float limitVoltage3 = 0.0f;
float setVoltage1   = 245.0f;
float setVoltage2   = 245.0f;
float setVoltage3   = 245.0f;

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
    if (time(nullptr) > 1700000000) {
      Serial.println("[NTP] time synced");
      return;
    }
    delay(500);
  }
  Serial.println("[NTP] warning: time may be wrong");
}

String apiPath(const char* suffix) {
  return String("/api/v1/") + DEVICE_TOKEN + suffix;
}

bool httpsExchange(const char* method, const String& path, const String& body, int* codeOut, String* respOut) {
  prepareTlsClient();
  HTTPClient http;
  String url = String("https://") + API_HOST + path;
  if (!http.begin(tlsClient, url)) {
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
    http.end();
    return false;
  }
  *respOut = http.getString();
  http.end();
  return true;
}

bool postClientJson(const String& jsonBody, int* codeOut) {
  String resp;
  return httpsExchange("POST", apiPath("/attributes"), jsonBody, codeOut, &resp);
}

void postClientOneFloat(const char* key, float value) {
  StaticJsonDocument<96> doc;
  doc[key] = value;
  String body;
  serializeJson(doc, body);
  int code = 0;
  if (postClientJson(body, &code)) {
    Serial.printf("[CLIENT ATTR] %s -> %d\n", key, code);
  }
}

float jsonVariantToFloat(JsonVariant v) {
  if (v.isNull()) return 0.f;
  if (v.is<float>() || v.is<double>()) return v.as<float>();
  if (v.is<bool>()) return v.as<bool>() ? 1.f : 0.f;
  if (v.is<int>() || v.is<long>()) return (float)v.as<int>();
  if (v.is<const char*>()) return strtof(v.as<const char*>(), nullptr);
  return 0.f;
}

void applyChannel(int ch, float raw) {
  if (ch < 1 || ch > 4) return;
  bool state = (raw > 0.5f);
  channelState[ch] = state;
  int pin = -1;
  switch (ch) {
    case 1: pin = RELAY_PIN_1; break;
    case 2: pin = RELAY_PIN_2; break;
    case 3: pin = RELAY_PIN_3; break;
    case 4: pin = RELAY_PIN_4; break;
  }
  if (pin >= 0) digitalWrite(pin, state ? HIGH : LOW);

  char ck[12];
  snprintf(ck, sizeof(ck), "channel%d", ch);
  postClientOneFloat(ck, state ? 1.0f : 0.0f);
  Serial.printf("[ATTR] channel%d = %s\n", ch, state ? "ON" : "OFF");
}

void processSharedAttribute(const char* key, float value) {
  Serial.printf("[ATTR] %s = %.2f\n", key, value);

  if (strcmp(key, "channel1") == 0) {
    applyChannel(1, value);
    return;
  }
  if (strcmp(key, "channel2") == 0) {
    applyChannel(2, value);
    return;
  }
  if (strcmp(key, "channel3") == 0) {
    applyChannel(3, value);
    return;
  }
  if (strcmp(key, "channel4") == 0) {
    applyChannel(4, value);
    return;
  }

  if (strcmp(key, "volume") == 0) {
    volume = value;
    postClientOneFloat("volume", volume);
    Serial.printf("[ATTR] volume = %.2f\n", volume);
    return;
  }

  if (strcmp(key, "limitVoltage1") == 0) {
    limitVoltage1 = value;
    setVoltage1   = value;
    postClientOneFloat("setVoltage1", setVoltage1);
    Serial.printf("[ATTR] limitVoltage1=%.2f → setVoltage1=%.2f\n", value, setVoltage1);
    return;
  }
  if (strcmp(key, "limitVoltage2") == 0) {
    limitVoltage2 = value;
    setVoltage2   = value;
    postClientOneFloat("setVoltage2", setVoltage2);
    Serial.printf("[ATTR] limitVoltage2=%.2f → setVoltage2=%.2f\n", value, setVoltage2);
    return;
  }
  if (strcmp(key, "limitVoltage3") == 0) {
    limitVoltage3 = value;
    setVoltage3   = value;
    postClientOneFloat("setVoltage3", setVoltage3);
    Serial.printf("[ATTR] limitVoltage3=%.2f → setVoltage3=%.2f\n", value, setVoltage3);
    return;
  }

  StaticJsonDocument<128> doc;
  doc[key] = value;
  String body;
  serializeJson(doc, body);
  int code = 0;
  postClientJson(body, &code);
  Serial.printf("[CLIENT ATTR] unknown shared key \"%s\" echoed -> %d\n", key, code);
}

#define MAX_TRACKED 24
struct TrackedShared {
  char  key[24];
  float val;
} tracked[MAX_TRACKED];
int nTracked = 0;

bool sharedServerValueChanged(const char* key, float serverVal) {
  for (int i = 0; i < nTracked; i++) {
    if (strncmp(tracked[i].key, key, sizeof(tracked[i].key)) == 0) {
      return fabsf(tracked[i].val - serverVal) > 1e-4f;
    }
  }
  return true;
}

void rememberSharedServerValue(const char* key, float serverVal) {
  for (int i = 0; i < nTracked; i++) {
    if (strncmp(tracked[i].key, key, sizeof(tracked[i].key)) == 0) {
      tracked[i].val = serverVal;
      return;
    }
  }
  if (nTracked >= MAX_TRACKED) return;
  strncpy(tracked[nTracked].key, key, sizeof(tracked[nTracked].key) - 1);
  tracked[nTracked].key[sizeof(tracked[nTracked].key) - 1] = 0;
  tracked[nTracked].val = serverVal;
  nTracked++;
}

void pollSharedAttributes() {
  int code = 0;
  String resp;
  String path = apiPath("/attributes/flat") + "?scope=SHARED";
  if (!httpsExchange("GET", path, "", &code, &resp)) {
    Serial.printf("[SHARED] request failed code=%d\n", code);
    return;
  }
  if (code != 200) {
    Serial.printf("[SHARED] HTTP %d\n", code);
    return;
  }

  DynamicJsonDocument doc(16384);
  DeserializationError err = deserializeJson(doc, resp);
  if (err) {
    Serial.println("[SHARED] JSON parse error");
    return;
  }

  JsonArray arr = doc.as<JsonArray>();
  if (arr.isNull()) {
    Serial.println("[SHARED] expected JSON array");
    return;
  }

  for (JsonObject item : arr) {
    const char* sc = item["scope"] | "";
    if (strcmp(sc, "SHARED") != 0) continue;
    const char* k = item["key"] | "";
    if (!k[0]) continue;
    float v = jsonVariantToFloat(item["value"]);
    if (!sharedServerValueChanged(k, v)) continue;
    processSharedAttribute(k, v);
    rememberSharedServerValue(k, v);
  }
}

unsigned long lastPoll         = 0;
unsigned long lastTelemetry    = 0;
unsigned long lastStatusPrint  = 0;

void setup() {
  Serial.begin(115200);
  delay(1500);

  pinMode(RELAY_PIN_1, OUTPUT);
  digitalWrite(RELAY_PIN_1, LOW);
  pinMode(RELAY_PIN_2, OUTPUT);
  digitalWrite(RELAY_PIN_2, LOW);
  pinMode(RELAY_PIN_3, OUTPUT);
  digitalWrite(RELAY_PIN_3, LOW);
  pinMode(RELAY_PIN_4, OUTPUT);
  digitalWrite(RELAY_PIN_4, LOW);

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

  pollSharedAttributes();

  Serial.println("[HTTP] AllFunctionTest_http started");
  Serial.println(
    "[HTTP] RPC: use AllFunctionTest_mqtt for dashboard RPC widget "
    "(ping, getStatus, getConfig, getDiagnostics, setValue, relay_set, "
    "reset, reboot, telemetry_burst).");
}

void loop() {
  unsigned long now = millis();

  if (now - lastPoll > POLL_SHARED_MS) {
    lastPoll = now;
    pollSharedAttributes();
  }

  if (now - lastTelemetry > 10000) {
    lastTelemetry = now;

    StaticJsonDocument<384> tel;
    tel["temperature"] = (float)random(20, 35);
    tel["humidity"]    = (float)random(40, 80);
    tel["current"]     = (float)random(1, 10);
    tel["power"]       = (float)random(100, 500);
    tel["voltage1"]    = (float)random(220, 280);
    tel["voltage2"]    = (float)random(220, 280);
    tel["voltage3"]    = (float)random(220, 280);
    String telBody;
    serializeJson(tel, telBody);
    int code = 0;
    String r;
    if (httpsExchange("POST", apiPath("/telemetry"), telBody, &code, &r)) {
      Serial.printf("[TELEMETRY] %d\n", code);
    }

    StaticJsonDocument<256> attrs;
    attrs["freeHeap"]   = ESP.getFreeHeap();
    attrs["wifiRSSI"]   = WiFi.RSSI();
    attrs["uptime"]     = millis() / 1000;
    attrs["sdkVersion"] = 1.0;
    String attrBody;
    serializeJson(attrs, attrBody);
    code = 0;
    if (httpsExchange("POST", apiPath("/attributes"), attrBody, &code, &r)) {
      Serial.printf("[CLIENT ATTR] %d\n", code);
    }
  }

  if (now - lastStatusPrint > 30000) {
    lastStatusPrint = now;
    Serial.printf(
      "[STATUS] https=%s heap=%u rssi=%d\n",
      WiFi.isConnected() ? "up" : "down",
      (unsigned)ESP.getFreeHeap(),
      WiFi.RSSI()
    );
  }

  delay(5);
}
