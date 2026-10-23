// =============================================================
// GpsTracker_mqtt — location / speed / ignition / fuel
//
// Keys match plant trips, geofences and fuel (customer guide):
//   latitude, longitude, speed_kmh, ignition, fuel_volume_l
//
// Simulated path around Delhi for lab demos. Replace the
// updateFix() body with a real GPS module when you have one.
//
// Copy secrets.h.example → secrets.h before flashing.
// =============================================================

#include <AutoconnectoSDK.h>
#include <math.h>
#include "secrets.h"

AutoconnectoSDK sdk;

// Start near New Delhi; walk a slow loop so trips can open/close.
float latitude = 28.6139f;
float longitude = 77.2090f;
float speedKmh = 0.0f;
int ignition = 0;
float fuelVolumeL = 40.0f;
unsigned long phaseStart = 0;

void updateFix(unsigned long now) {
  const unsigned long phase = (now / 1000UL) % 120UL; // 2-minute cycle
  if (phase < 15) {
    // Idle, ignition off — trip should close / stay closed.
    ignition = 0;
    speedKmh = 0.0f;
  } else if (phase < 25) {
    ignition = 1;
    speedKmh = 0.0f; // idle with ignition on
  } else if (phase < 100) {
    ignition = 1;
    speedKmh = 35.0f + (phase % 10);
    // ~0.0003 deg ≈ 30 m step each publish (~35 s) — enough for a short trip.
    const float heading = (now / 1000.0f) * 0.02f;
    latitude += cosf(heading) * 0.00025f;
    longitude += sinf(heading) * 0.00025f;
    fuelVolumeL = fuelVolumeL > 5.02f ? fuelVolumeL - 0.02f : 5.0f;
  } else {
    ignition = 1;
    speedKmh = 0.0f; // stop before ignition off next cycle
  }
}

void setup() {
  Serial.begin(115200);
  delay(1500);

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

  sdk.begin(config);
  phaseStart = millis();
  Serial.println("[SDK] GpsTracker_mqtt started");
}

unsigned long lastTel = 0;

void loop() {
  sdk.loop();
  const unsigned long now = millis();
  if (now - lastTel < 35000UL) {
    delay(10);
    return;
  }
  lastTel = now;
  updateFix(now);

  JsonDocument tel;
  tel["latitude"] = latitude;
  tel["longitude"] = longitude;
  tel["speed_kmh"] = speedKmh;
  tel["speed"] = speedKmh; // alias accepted by trip motion
  tel["ignition"] = ignition;
  tel["fuel_volume_l"] = fuelVolumeL;

  const bool ok = sdk.sendTelemetry(tel);
  Serial.printf(
    "[TEL] ok=%d lat=%.5f lon=%.5f speed=%.0f ign=%d fuel=%.1f\n",
    ok,
    latitude,
    longitude,
    speedKmh,
    ignition,
    fuelVolumeL
  );
  delay(10);
}
