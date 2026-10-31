// =============================================================
// WatchdogReconnect_mqtt — Autoconnecto SDK example (MQTT)
//
// PURPOSE
//   The SDK (v1.7.85+) already rejoins Wi‑Fi after repeated MQTT
//   failures and calls esp_restart() after ~3 minutes down
//   (mqttRestartAfterDisconnectMs). This sketch adds a longer
//   sketch-level safety net (default 5 minutes) and prints [NET]
//   lines so you can watch recovery. Prefer BOARD_VERIFY B8
//   (pull router power ~60 s) to prove library recovery.
// =============================================================

#include <AutoconnectoSDK.h>
#include "secrets.h"

AutoconnectoSDK sdk;

static const unsigned long MAX_DISCONNECT_MS = 5UL * 60UL * 1000UL; // 5 minutes
static unsigned long disconnectedSince = 0;

void onConnect(bool connected) {
  (void)connected;
  disconnectedSince = 0;
  Serial.println("[NET] Connected");
}

void onDisconnect(bool connected) {
  (void)connected;
  if (disconnectedSince == 0) {
    disconnectedSince = millis();
  }
  Serial.println("[NET] Disconnected");
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

  sdk.onConnect(onConnect);
  sdk.onDisconnect(onDisconnect);
  sdk.begin(config);

  Serial.println("[SDK] WatchdogReconnect_mqtt started");
}

unsigned long lastTelemetry = 0;

void loop() {
  sdk.loop();

  unsigned long now = millis();

  if (sdk.connected()) {
    // small heartbeat telemetry
    if (now - lastTelemetry > 15000) {
      lastTelemetry = now;
      sdk.sendTelemetry("uptime", (float)(now / 1000));
    }
  } else {
    if (disconnectedSince != 0 && (now - disconnectedSince) > MAX_DISCONNECT_MS) {
      Serial.println("[WDT] Disconnected too long — restarting");
      delay(200);
      ESP.restart();
    }
  }

  delay(10);
}

