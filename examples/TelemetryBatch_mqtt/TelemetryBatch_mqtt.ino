// =============================================================
// TelemetryBatch_mqtt — Autoconnecto SDK example (MQTT)
//
// PURPOSE
//   Demonstrates a simple buffer-and-flush pattern over MQTT:
//   buffer telemetry locally and then publish multiple samples
//   sequentially when online.
//
// NOTE
//   MQTT ingress is per-message JSON object (no single "batch"
//   payload contract). This example flushes a batch by sending
//   N individual telemetry publishes.
//
//   Growth and Enterprise store one reading per second. A closer
//   gap is not stored. MQTT does not return that refusal, so this
//   flush waits 1100 ms between publishes. A top-level "ts" is the
//   reading time and is not stored as a key.
// =============================================================

#include <AutoconnectoSDK.h>
#include "secrets.h"

AutoconnectoSDK sdk;

struct Sample {
  float temperature;
  float humidity;
  float current;
  float power;
  float voltage1;
  float voltage2;
  float voltage3;
};

static const int MAX_SAMPLES = 20;
Sample buf[MAX_SAMPLES];
int nBuf = 0;

void pushSample() {
  if (nBuf >= MAX_SAMPLES) return;
  buf[nBuf++] = Sample{
    (float)random(20, 35),
    (float)random(40, 80),
    (float)random(1, 10),
    (float)random(100, 500),
    (float)random(220, 280),
    (float)random(220, 280),
    (float)random(220, 280),
  };
}

void flushSamples() {
  if (!sdk.connected()) return;
  if (nBuf == 0) return;

  Serial.printf("[BATCH] flushing %d samples\n", nBuf);

  for (int i = 0; i < nBuf; i++) {
    StaticJsonDocument<256> tel;
    tel["temperature"] = buf[i].temperature;
    tel["humidity"]    = buf[i].humidity;
    tel["current"]     = buf[i].current;
    tel["power"]       = buf[i].power;
    tel["voltage1"]    = buf[i].voltage1;
    tel["voltage2"]    = buf[i].voltage2;
    tel["voltage3"]    = buf[i].voltage3;
    if (!sdk.sendTelemetry(tel)) {
      Serial.printf("[BATCH] publish refused sample %d\n", i);
    } else {
      Serial.printf("[BATCH] sent sample %d/%d\n", i + 1, nBuf);
    }
    delay(1100);
  }

  nBuf = 0;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  SDKConfig config;
  config.wifiSSID = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;
  config.mqttHost     = "mqtt.autoconnecto.in";
  config.mqttPort     = 8883;
  config.wssPort      = 8084;
  config.deviceToken = DEVICE_TOKEN;
  config.enableWS     = true;
  config.enableMQTT   = true;
  config.allowInsecureTLS = false;
  config.rootCA       = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = true;

  sdk.begin(config);

  Serial.println("[SDK] TelemetryBatch_mqtt started");
}

unsigned long lastSample = 0;
unsigned long lastFlush  = 0;

void loop() {
  sdk.loop();

  unsigned long now = millis();

  // Buffer a new reading every 2 seconds
  if (now - lastSample > 2000) {
    lastSample = now;
    pushSample();
    Serial.printf("[BATCH] buffered=%d\n", nBuf);
  }

  // Flush every 15 seconds if connected
  if (now - lastFlush > 15000) {
    lastFlush = now;
    flushSamples();
  }

  delay(10);
}

