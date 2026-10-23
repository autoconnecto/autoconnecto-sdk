// =============================================================
// DeepSleep_http — wake → POST telemetry → deep sleep
//
// Battery pattern. Default inactivity timeout is 5 minutes —
// set the device's inactivity timeout above SLEEP_SEC so the
// platform does not flap INACTIVE while the MCU sleeps.
//
// Copy secrets.h.example → secrets.h before flashing.
// =============================================================

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <esp_sleep.h>
#include <AutoconnectoIsrgRoots.h>
#include "secrets.h"

void goToSleep() {
  Serial.printf("[SLEEP] %d s\n", (int)SLEEP_SEC);
  Serial.flush();
  esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_SEC * 1000000ULL);
  esp_deep_sleep_start();
}

bool postTelemetry(float temperature, float humidity) {
  WiFiClientSecure client;
  client.setCACert(AUTOCONNECTO_ROOT_CA);

  String url = String("https://api.autoconnecto.in/api/v1/") +
               DEVICE_TOKEN + "/telemetry";
  HTTPClient http;
  if (!http.begin(client, url)) {
    Serial.println("[HTTP] begin failed");
    return false;
  }
  http.addHeader("Content-Type", "application/json");
  String body = String("{\"temperature\":") + temperature +
                ",\"humidity\":" + humidity +
                ",\"ts\":" + String((uint32_t)(millis())) + "}";
  // Prefer wall-clock when RTC is set; millis is fine for demo.
  const int code = http.POST(body);
  Serial.printf("[HTTP] POST %d\n", code);
  http.end();
  return code >= 200 && code < 300;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("[SDK] DeepSleep_http wake");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) delay(500);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[NET] Wi-Fi failed — sleep anyway");
    goToSleep();
  }

  float t = 20.0f + (esp_random() % 10);
  float h = 45.0f + (esp_random() % 20);
  postTelemetry(t, h);
  delay(200);
  WiFi.disconnect(true);
  goToSleep();
}

void loop() {
  // not used — deep sleep restarts at setup()
}
