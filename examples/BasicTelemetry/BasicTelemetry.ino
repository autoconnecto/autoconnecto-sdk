// =============================================================
// BasicTelemetry — Autoconnecto SDK example
//
// PURPOSE
//   Simplest possible sketch. Connect to the platform and send
//   sensor readings every 10 seconds. No attributes, no RPC.
//   Use this first to verify your device token and connection.
//
// DASHBOARD WIDGETS THAT WORK WITH THIS SKETCH
//   - Any chart/telemetry widget configured with:
//     temperature, humidity, current, power,
//     voltage1, voltage2, voltage3
//
// KEYS USED (same across all examples — one dashboard for all)
//   Telemetry : temperature, humidity, current, power,
//               voltage1, voltage2, voltage3
//   Client    : freeHeap, wifiRSSI, uptime, sdkVersion
// =============================================================

#include <AutoconnectoSDK.h>

AutoconnectoSDK sdk;

// ------------------------------------------------------------
// Root CAs — Let's Encrypt ISRG Root X1 + ISRG Root X2
//
// Multi-CA trust bundle (required when allowInsecureTLS = false).
// mbedtls/esp_tls accept concatenated PEM blocks; the chain is valid
// if it terminates at ANY root below. This makes devices resilient
// across LE intermediate rotation and the X1 → X2 transition.
//
//   ISRG Root X1: RSA 4096, valid → 2035-06-04 (default chain today)
//   ISRG Root X2: ECDSA P-384, valid → 2040-09-17 (future ECDSA chain)
//
// https://letsencrypt.org/certificates/
// ------------------------------------------------------------
static const char* AUTOCONNECTO_ROOT_CA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIICGzCCAaGgAwIBAgIQQdKd0XLq7qeAwSxs6S+HUjAKBggqhkjOPQQDAzBPMQsw
CQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFyY2gg
R3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBYMjAeFw0yMDA5MDQwMDAwMDBaFw00
MDA5MTcxNjAwMDBaME8xCzAJBgNVBAYTAlVTMSkwJwYDVQQKEyBJbnRlcm5ldCBT
ZWN1cml0eSBSZXNlYXJjaCBHcm91cDEVMBMGA1UEAxMMSVNSRyBSb290IFgyMHYw
EAYHKoZIzj0CAQYFK4EEACIDYgAEzZvVn4CDCuwJSvMWSj5cz3es3mcFDR0HttwW
+1qLFNvicWDEukWVEYmO6gbf9yoWHKS5xcUy4APgHoIYOIvXRdgKam7mAHf7AlF9
ItgKbppbd9/w+kHsOdx1ymgHDB/qo0IwQDAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0T
AQH/BAUwAwEB/zAdBgNVHQ4EFgQUfEKWrt5LSDv6kviejM9ti6lyN5UwCgYIKoZI
zj0EAwMDaAAwZQIwe3lORlCEwkSHRhtFcP9Ymd70/aTSVaYgLXTWNLxBo1BfASdW
tL4ndQavEi51mI38AjEAi/V3bNTIZargCyzuFJ0nN6T5U6VR5CmD1/iQMVtCnwr1
/q4AaOeMSQ+2b1tbFfLn
-----END CERTIFICATE-----
)EOF";

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(2000);

  SDKConfig config;

  // ---- WiFi ----
  config.wifiSSID     = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";

  // ---- MQTT / WSS ----
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort  = 8084;

  // ---- Device ----
  config.deviceToken = "YOUR_DEVICE_TOKEN";

  // ---- Transport ----
  config.enableWS   = true;
  config.enableMQTT = true;

  // ---- TLS ----
  config.allowInsecureTLS = false;
  config.rootCA           = AUTOCONNECTO_ROOT_CA;

  // ---- Debug ----
  config.enableSerialLogs = true;

  sdk.begin(config);

  Serial.println("[SDK] BasicTelemetry started");
}

// ============================================================
// LOOP
// ============================================================

unsigned long lastTelemetry   = 0;
unsigned long lastAttrReport  = 0;

void loop() {

  // CRITICAL: must run every iteration — services MQTT keepalive.
  // Never delay() more than ~50ms here.
  sdk.loop();

  unsigned long now = millis();

  // ---- Send telemetry every 10 seconds ----
  if (now - lastTelemetry > 10000) {
    lastTelemetry = now;

    // Single-key calls (up to 3 keys per variadic call)
    sdk.sendTelemetry("temperature", (float)random(20, 35));
    sdk.sendTelemetry(
      "humidity", (float)random(40, 80),
      "current",  (float)random(1, 10),
      "power",    (float)random(100, 500)
    );

    // 4+ keys require a JsonDocument
    StaticJsonDocument<128> voltages;
    voltages["voltage1"] = (float)random(220, 280);
    voltages["voltage2"] = (float)random(220, 280);
    voltages["voltage3"] = (float)random(220, 280);
    sdk.sendTelemetry(voltages);

    Serial.println("[TEL] Sent telemetry");
  }

  // ---- Report device health as client attributes every 30 seconds ----
  if (now - lastAttrReport > 30000) {
    lastAttrReport = now;

    StaticJsonDocument<128> attrs;
    attrs["freeHeap"]   = ESP.getFreeHeap();
    attrs["wifiRSSI"]   = WiFi.RSSI();
    attrs["uptime"]     = millis() / 1000;
    attrs["sdkVersion"] = 1.0;
    sdk.sendClientAttributes(attrs);

    Serial.println("[ATTR] Sent device health");
  }

  delay(10);
}
