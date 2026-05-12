// =============================================================
// SwitchControl — Autoconnecto SDK example
//
// PURPOSE
//   Demonstrates the shared attribute → hardware → client
//   attribute feedback loop. This is the core mechanism that
//   keeps dashboard widgets in sync with device state across
//   power cycles without any user action.
//
// DASHBOARD WIDGETS THAT WORK WITH THIS SKETCH
//   - Switch widget      : channel1, channel2, channel3, channel4
//   - SliderControl      : volume
//   - AttributeControlCard : limitVoltage1, limitVoltage2, limitVoltage3
//
// HOW THE SYNC WORKS
//   1. Dashboard writes a shared attribute (e.g. channel1 = 1)
//   2. onAttributeUpdate() fires on the device
//   3. Device applies the value to hardware (relay ON)
//   4. Device sends a client attribute confirmation (channel1 = 1)
//   5. Dashboard reads the client attribute — confirms hardware state
//
//   On power cycle: requestSharedAttributes() replays all stored
//   shared attributes through onAttributeUpdate() → device
//   rebuilds full state from platform without user intervention.
//
// KEYS USED (same across all examples — one dashboard for all)
//   Shared attrs (in) : channel1, channel2, channel3, channel4,
//                       volume, limitVoltage1/2/3
//   Client attrs (out): channel1, channel2, channel3, channel4,
//                       volume, setVoltage1/2/3,
//                       freeHeap, wifiRSSI, uptime, sdkVersion
//   Telemetry         : temperature, humidity, current, power,
//                       voltage1, voltage2, voltage3
// =============================================================

#include <AutoconnectoSDK.h>

AutoconnectoSDK sdk;

// ------------------------------------------------------------
// Root CAs — Let's Encrypt ISRG Root X1 + ISRG Root X2
// Multi-CA trust bundle so devices remain valid across LE's
// X1 (RSA → 2035) and X2 (ECDSA → 2040) root rollout.
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
// HARDWARE PINS — adjust to your wiring
// ============================================================

#define RELAY_PIN_1   2
#define RELAY_PIN_2   4
#define RELAY_PIN_3   5
#define RELAY_PIN_4   18

// ============================================================
// DEVICE STATE
// ============================================================

bool  channelState[5] = {false, false, false, false, false}; // index 0 unused
float volume          = 0.0f;
float limitVoltage1   = 0.0f;
float limitVoltage2   = 0.0f;
float limitVoltage3   = 0.0f;
float setVoltage1     = 245.0f;
float setVoltage2     = 245.0f;
float setVoltage3     = 245.0f;

// ============================================================
// HELPER — drive relay, update state, confirm via client attr
// ============================================================

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

  // Confirm back to dashboard — closes the feedback loop.
  sdk.sendClientAttribute("channel" + String(ch), state ? 1.0f : 0.0f);
  Serial.printf("[ATTR] channel%d = %s\n", ch, state ? "ON" : "OFF");
}

// ============================================================
// ATTRIBUTE CALLBACK
//
// Fires for BOTH live dashboard writes AND requestSharedAttributes()
// responses at startup. Handle identically — apply + confirm.
// ============================================================

void onAttributeUpdate(const String& key, float value) {
  Serial.printf("[ATTR] %s = %.2f\n", key.c_str(), value);

  // Switch widget channels
  if (key == "channel1") { applyChannel(1, value); return; }
  if (key == "channel2") { applyChannel(2, value); return; }
  if (key == "channel3") { applyChannel(3, value); return; }
  if (key == "channel4") { applyChannel(4, value); return; }

  // SliderControl widget
  if (key == "volume") {
    volume = value;
    sdk.sendClientAttribute("volume", volume);
    return;
  }

  // AttributeControlCard widgets — store limit, apply to working copy,
  // confirm back so dashboard shows current setVoltageN value.
  if (key == "limitVoltage1") {
    limitVoltage1 = value;
    setVoltage1   = value;
    sdk.sendClientAttribute("setVoltage1", setVoltage1);
    return;
  }
  if (key == "limitVoltage2") {
    limitVoltage2 = value;
    setVoltage2   = value;
    sdk.sendClientAttribute("setVoltage2", setVoltage2);
    return;
  }
  if (key == "limitVoltage3") {
    limitVoltage3 = value;
    setVoltage3   = value;
    sdk.sendClientAttribute("setVoltage3", setVoltage3);
    return;
  }

  // Generic fallthrough — any future widget key auto-confirmed.
  sdk.sendClientAttribute(key, value);
}

// ============================================================
// CONNECTION CALLBACK — re-sync on every (re)connect
// ============================================================

void onConnect(bool connected) {
  Serial.println("[NET] Connected — requesting shared attributes");
  sdk.requestSharedAttributes();
}

void onDisconnect(bool connected) {
  Serial.println("[NET] Disconnected");
}

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(RELAY_PIN_1, OUTPUT); digitalWrite(RELAY_PIN_1, LOW);
  pinMode(RELAY_PIN_2, OUTPUT); digitalWrite(RELAY_PIN_2, LOW);
  pinMode(RELAY_PIN_3, OUTPUT); digitalWrite(RELAY_PIN_3, LOW);
  pinMode(RELAY_PIN_4, OUTPUT); digitalWrite(RELAY_PIN_4, LOW);

  SDKConfig config;
  config.wifiSSID         = "YOUR_WIFI_SSID";
  config.wifiPassword     = "YOUR_WIFI_PASSWORD";
  config.mqttHost         = "mqtt.autoconnecto.in";
  config.mqttPort         = 8883;
  config.wssPort          = 8084;
  config.deviceToken      = "YOUR_DEVICE_TOKEN";
  config.enableWS         = true;
  config.enableMQTT       = true;
  config.allowInsecureTLS = false;
  config.rootCA           = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = true;

  sdk.onAttributeUpdate(onAttributeUpdate);
  sdk.onConnect(onConnect);
  sdk.onDisconnect(onDisconnect);

  sdk.begin(config);

  // Restore full device state from platform at startup.
  // onAttributeUpdate fires for every stored shared attribute.
  sdk.requestSharedAttributes();

  Serial.println("[SDK] SwitchControl started");
}

// ============================================================
// LOOP
// ============================================================

unsigned long lastTelemetry  = 0;
unsigned long lastAttrReport = 0;

void loop() {
  sdk.loop();

  unsigned long now = millis();

  // Telemetry every 10 seconds
  if (now - lastTelemetry > 10000) {
    lastTelemetry = now;

    sdk.sendTelemetry("temperature", (float)random(20, 35));
    sdk.sendTelemetry(
      "humidity", (float)random(40, 80),
      "current",  (float)random(1, 10),
      "power",    (float)random(100, 500)
    );

    StaticJsonDocument<128> voltages;
    voltages["voltage1"] = (float)random(220, 280);
    voltages["voltage2"] = (float)random(220, 280);
    voltages["voltage3"] = (float)random(220, 280);
    sdk.sendTelemetry(voltages);
  }

  // Device health every 30 seconds
  if (now - lastAttrReport > 30000) {
    lastAttrReport = now;

    StaticJsonDocument<128> attrs;
    attrs["freeHeap"]   = ESP.getFreeHeap();
    attrs["wifiRSSI"]   = WiFi.RSSI();
    attrs["uptime"]     = millis() / 1000;
    attrs["sdkVersion"] = 1.0;
    sdk.sendClientAttributes(attrs);
  }

  delay(10);
}
