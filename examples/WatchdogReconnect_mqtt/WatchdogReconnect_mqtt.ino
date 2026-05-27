// =============================================================
// WatchdogReconnect_mqtt — Autoconnecto SDK example (MQTT)
//
// PURPOSE
//   Demonstrates a conservative "don't hang forever" pattern:
//   - runs the SDK loop frequently
//   - if disconnected for too long, restarts the device
//
// This is useful on unstable networks where cellular/WiFi drops
// can wedge the system. Keep the thresholds conservative.
// =============================================================

#include <AutoconnectoSDK.h>

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
  config.wifiSSID     = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";
  config.mqttHost     = "mqtt.autoconnecto.in";
  config.mqttPort     = 8883;
  config.wssPort      = 8084;
  config.deviceToken  = "YOUR_DEVICE_TOKEN";
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

