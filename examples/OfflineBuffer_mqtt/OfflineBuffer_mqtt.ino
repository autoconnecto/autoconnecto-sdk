// =============================================================
// OfflineBuffer_mqtt — keep readings across an outage
//
// Why: when MQTT is down, sendTelemetry returns false and the
// sample is lost (board verify: drop_network lost one reading).
// This sketch queues samples in RAM (+ LittleFS ring) with a
// real Unix timestamp (NTP), then flushes after reconnect.
//
// Copy secrets.h.example → secrets.h before flashing.
// =============================================================

#include <LittleFS.h>
#include <time.h>
#include <AutoconnectoSDK.h>
#include "secrets.h"

AutoconnectoSDK sdk;

struct Sample {
  uint32_t tsSec; // Unix seconds (UTC)
  float temperature;
  float humidity;
};

static const int MAX_RAM = 32;
static const char* QUEUE_PATH = "/ota/offline_q.bin";
Sample ram[MAX_RAM];
int nRam = 0;
bool wasConnected = false;
bool timeOk = false;

bool ensureTime() {
  if (timeOk) {
    time_t now = time(nullptr);
    if (now > 1700000000) return true; // after 2023
  }
  // UTC — platform stores ms epoch; dashboards apply tenant TZ.
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  for (int i = 0; i < 40; i++) {
    time_t now = time(nullptr);
    if (now > 1700000000) {
      timeOk = true;
      Serial.printf("[TIME] synced unix=%lu\n", (unsigned long)now);
      return true;
    }
    delay(250);
  }
  Serial.println("[TIME] NTP not ready — will retry; samples need wall clock");
  return false;
}

uint32_t wallClockSec() {
  time_t now = time(nullptr);
  if (now > 1700000000) return (uint32_t)now;
  return 0;
}

void persistRam() {
  if (!LittleFS.begin(true)) return;
  File f = LittleFS.open(QUEUE_PATH, "w");
  if (!f) return;
  f.write((uint8_t*)&nRam, sizeof(nRam));
  if (nRam > 0) f.write((uint8_t*)ram, sizeof(Sample) * nRam);
  f.close();
}

void loadPersisted() {
  if (!LittleFS.begin(true)) return;
  if (!LittleFS.exists(QUEUE_PATH)) return;
  File f = LittleFS.open(QUEUE_PATH, "r");
  if (!f) return;
  int n = 0;
  if (f.read((uint8_t*)&n, sizeof(n)) != sizeof(n)) {
    f.close();
    return;
  }
  if (n < 0 || n > MAX_RAM) {
    f.close();
    return;
  }
  if (n && f.read((uint8_t*)ram, sizeof(Sample) * n) != (int)(sizeof(Sample) * n)) {
    f.close();
    return;
  }
  nRam = n;
  f.close();
  Serial.printf("[OFFLINE] loaded %d queued sample(s)\n", nRam);
}

void enqueue(float temperature, float humidity) {
  uint32_t ts = wallClockSec();
  if (!ts) {
    Serial.println("[OFFLINE] skip queue — no NTP time yet (avoids 1970 timestamps)");
    return;
  }
  if (nRam >= MAX_RAM) {
    memmove(&ram[0], &ram[1], sizeof(Sample) * (MAX_RAM - 1));
    nRam = MAX_RAM - 1;
  }
  ram[nRam++] = Sample{ ts, temperature, humidity };
  persistRam();
  Serial.printf("[OFFLINE] queued n=%d ts=%lu\n", nRam, (unsigned long)ts);
}

void flushQueue() {
  if (!sdk.connected() || nRam == 0) return;
  Serial.printf("[OFFLINE] flushing %d\n", nRam);
  for (int i = 0; i < nRam; i++) {
    JsonDocument tel;
    tel["temperature"] = ram[i].temperature;
    tel["humidity"] = ram[i].humidity;
    // Top-level ts is reading time in ms (Unix epoch).
    tel["ts"] = (uint64_t)ram[i].tsSec * 1000ULL;
    if (!sdk.sendTelemetry(tel)) {
      Serial.printf("[OFFLINE] flush stopped at %d\n", i);
      int left = nRam - i;
      memmove(&ram[0], &ram[i], sizeof(Sample) * left);
      nRam = left;
      persistRam();
      return;
    }
    delay(1100); // plan-safe gap on Growth/Enterprise
  }
  nRam = 0;
  persistRam();
  Serial.println("[OFFLINE] queue empty");
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  loadPersisted();

  SDKConfig config;
  config.wifiSSID = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;
  config.deviceToken = DEVICE_TOKEN;
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort = 8084;
  config.enableWS = true;
  config.enableMQTT = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = false;

  sdk.onConnect([](bool) {
    Serial.println("[NET] connected — sync time then flush");
    ensureTime();
    flushQueue();
  });
  sdk.begin(config);
  Serial.println("[SDK] OfflineBuffer_mqtt started");
}

unsigned long lastSample = 0;

void loop() {
  sdk.loop();
  const bool online = sdk.connected();
  if (online) {
    ensureTime();
    if (!wasConnected) flushQueue();
  }
  wasConnected = online;

  if (millis() - lastSample < 35000UL) {
    delay(10);
    return;
  }
  lastSample = millis();

  float t = 20.0f + (millis() / 1000UL % 10);
  float h = 50.0f;
  if (online) {
    JsonDocument tel;
    tel["temperature"] = t;
    tel["humidity"] = h;
    if (!sdk.sendTelemetry(tel)) {
      Serial.println("[TEL] send failed — queue");
      enqueue(t, h);
    } else {
      Serial.println("[TEL] sent live");
    }
  } else {
    Serial.println("[TEL] offline — queue");
    enqueue(t, h);
  }
  delay(10);
}
