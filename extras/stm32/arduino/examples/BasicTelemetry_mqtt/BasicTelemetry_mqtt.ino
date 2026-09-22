// =============================================================
// STM32 Arduino-core template — BasicTelemetry_mqtt
//
// Goal: publish the same telemetry keys as the ESP32 example using:
//   devices/{token}/telemetry  (MQTTS :8883)
//
// This is a TEMPLATE because STM32 Arduino networking differs per board:
// - Ethernet (recommended in industry): W5500 / LAN8742A / built-in MAC
// - WiFi modules: core-dependent
//
// You must provide:
// - a network client (Ethernet/WiFi)
// - an MQTT client that supports TLS (or a TLS socket wrapped client)
// - the ISRG X1+X2 CA bundle (see: ../../../../raspberrypi/ca/isrg_roots.pem)
//
// If you get stuck, start with HTTPS GET https://api.autoconnecto.in/healthz
// to validate TLS + time sync first.
// =============================================================

// ----- Pick your network stack (examples) -----
// #include <Ethernet.h>   // W5x00 Ethernet shield/module
// #include <WiFi.h>       // if your STM32 core provides it

// ----- MQTT client (example only) -----
// Many STM32 Arduino users start with PubSubClient, but TLS support depends
// on the underlying Client implementation.
// #include <PubSubClient.h>

// ----- TLS client (example only) -----
// Replace with whatever secure client your STM32 core provides.
// #include <YourTlsClient.h>

// ------------ Edit these ------------
static const char* DEVICE_TOKEN = "YOUR_DEVICE_TOKEN";
static const char* MQTT_HOST    = "mqtt.autoconnecto.in";
static const int   MQTT_PORT    = 8883;

// ------------ Topic helper ------------
String telemetryTopic() {
  return String("devices/") + DEVICE_TOKEN + "/telemetry";
}

// ------------ TODO: create network + TLS + MQTT clients ------------
// Example shape:
// EthernetClient net;
// YourTlsClient  tls(net);
// PubSubClient   mqtt(tls);

bool mqttConnect() {
  // mqtt.setServer(MQTT_HOST, MQTT_PORT);
  // mqtt.setKeepAlive(60);
  // mqtt.setSocketTimeout(20);
  //
  // IMPORTANT: device token is BOTH username and clientId (matches ESP32 SDK)
  // return mqtt.connect(DEVICE_TOKEN, DEVICE_TOKEN, /*password*/ nullptr);
  return false; // TODO
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  // 1) Bring up network here (Ethernet/WiFi)
  // 2) Configure TLS root CA:
  //    - Use the ISRG X1+X2 bundle from raspberrypi/ca/isrg_roots.pem
  //    - Your TLS stack may require correct system time

  // 3) Connect MQTT
  Serial.println("[STM32] Starting BasicTelemetry_mqtt template");
  if (!mqttConnect()) {
    Serial.println("[MQTT] connect failed (template)");
  }
}

unsigned long lastTelemetry = 0;

void loop() {
  // mqtt.loop(); // must be called frequently

  unsigned long now = millis();
  if (now - lastTelemetry > 10000) {
    lastTelemetry = now;

    // Build JSON body (flat object)
    String payload =
      String("{") +
      "\"temperature\":" + String(random(20, 35)) + "," +
      "\"humidity\":" + String(random(40, 80)) + "," +
      "\"current\":" + String(random(1, 10)) + "," +
      "\"power\":" + String(random(100, 500)) + "," +
      "\"voltage1\":" + String(random(220, 280)) + "," +
      "\"voltage2\":" + String(random(220, 280)) + "," +
      "\"voltage3\":" + String(random(220, 280)) +
      "}";

    // mqtt.publish(telemetryTopic().c_str(), payload.c_str(), /*retained*/ false);
    Serial.println("[TEL] (template) would publish: " + payload);
  }

  delay(10);
}

