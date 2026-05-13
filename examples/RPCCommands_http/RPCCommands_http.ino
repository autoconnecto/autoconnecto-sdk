// =============================================================
// RPCCommands_http — HTTPS device-token API (ESP32)
//
// Pair sketch: examples/RPCCommands_mqtt/RPCCommands_mqtt.ino
//
// IMPORTANT
//   Dashboard RPC commands are delivered over MQTT / WSS to the
//   SDK, not over the device-token HTTPS API. This sketch cannot
//   receive ping, relay_set, reboot, etc. from the RPC widget.
//
//   It sends the SAME telemetry and periodic client attributes as
//   RPCCommands_mqtt so charts and health widgets behave the same.
//   For relay + shared-attribute control over HTTPS use
//   SwitchControl_http or AllFunctionTest_http.
//
// DEPENDENCIES
//   ArduinoJson
//   AutoconnectoSDK library — for `AutoconnectoIsrgRoots.h` (TLS roots) only.
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

#define RELAY_PIN_1 2
#define RELAY_PIN_2 4
#define RELAY_PIN_3 5
#define RELAY_PIN_4 18

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

unsigned long lastTelemetry  = 0;
unsigned long lastAttrReport = 0;

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

  Serial.println("[HTTP] RPCCommands_http — RPC widget requires MQTT (see header)");
}

void loop() {
  unsigned long now = millis();

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
    String body;
    serializeJson(tel, body);
    int code = 0;
    String r;
    if (httpsExchange("POST", apiPath("/telemetry"), body, &code, &r)) {
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
    String r;
    httpsExchange("POST", apiPath("/attributes"), body, &code, &r);
    Serial.printf("[CLIENT ATTR] %d\n", code);
  }

  delay(50);
}
