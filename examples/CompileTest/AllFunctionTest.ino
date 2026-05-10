#include <AutoconnectoSDK.h>

AutoconnectoSDK sdk;

// ============================================================
// ROOT CA (Let's Encrypt ISRG Root X1)
// ============================================================

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
)EOF";

// ============================================================
// HARDWARE PINS
// Adjust to match your actual wiring.
// ============================================================

#define RELAY_PIN_1   2
#define RELAY_PIN_2   4
#define RELAY_PIN_3   5
#define RELAY_PIN_4   18

// ============================================================
// DEVICE STATE
//
// channelState[] — Switch widget: channel1..channel4
// volume         — SliderControl widget (e.g. audio volume)
// limitVoltage1/2/3 — AttributeControlCard widgets
//                     Dashboard writes these as shared attrs.
//                     Device applies them and confirms back
//                     as client attrs with the same key.
// setVoltage1/2/3   — Local working copies; default 245V.
//                     Updated when limitVoltageN arrives.
//
// All state is rebuilt from shared attributes on every
// power cycle via requestSharedAttributes() → onAttributeUpdate().
// ============================================================

bool  channelState[5] = {false, false, false, false, false}; // index 0 unused

float volume        = 0.0f;

float limitVoltage1 = 0.0f;
float limitVoltage2 = 0.0f;
float limitVoltage3 = 0.0f;

float setVoltage1   = 245.0f; // operational default
float setVoltage2   = 245.0f;
float setVoltage3   = 245.0f;

// ============================================================
// HELPERS
// ============================================================

// Drive a relay channel, update state, and confirm via client attr.
// Called from onAttributeUpdate (live dashboard command) and
// on startup via requestSharedAttributes (power-cycle sync).
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

  if (pin >= 0) {
    digitalWrite(pin, state ? HIGH : LOW);
  }

  // Confirm back to dashboard as client attribute.
  // Feedback loop: shared attr → device → client attr.
  sdk.sendClientAttribute("channel" + String(ch), state ? 1.0f : 0.0f);

  Serial.printf("[ATTR] channel%d = %s\n", ch, state ? "ON" : "OFF");
}

// ============================================================
// ATTRIBUTE CALLBACK
//
// Called when:
//   1. Dashboard writes a shared attribute (live command)
//   2. requestSharedAttributes() response arrives at startup
//
// Both cases are handled identically — apply the value and
// confirm via client attribute. This is the core of the
// power-cycle sync mechanism.
// ============================================================

void onAttributeUpdate(const String& key, float value) {

  Serial.printf("[ATTR] %s = %.2f\n", key.c_str(), value);

  // ---- Switch widget channels ----
  if (key == "channel1") { applyChannel(1, value); return; }
  if (key == "channel2") { applyChannel(2, value); return; }
  if (key == "channel3") { applyChannel(3, value); return; }
  if (key == "channel4") { applyChannel(4, value); return; }

  // ---- SliderControl widget ----
  if (key == "volume") {
    volume = value;
    // Apply to hardware here (e.g. analogWrite for PWM volume)
    sdk.sendClientAttribute("volume", volume);
    Serial.printf("[ATTR] volume = %.2f\n", value);
    return;
  }

  // ---- AttributeControlCard widgets (voltage limits) ----
  // Shared attr key  : limitVoltageN  (dashboard writes this)
  // Client attr key  : setVoltageN    (device confirms the applied value)
  // Two different keys by design: "what the user set" vs "what the device is using".
  if (key == "limitVoltage1") {
    limitVoltage1 = value;
    setVoltage1   = value;
    sdk.sendClientAttribute("setVoltage1", setVoltage1);
    Serial.printf("[ATTR] limitVoltage1=%.2f → setVoltage1=%.2f\n", value, setVoltage1);
    return;
  }

  if (key == "limitVoltage2") {
    limitVoltage2 = value;
    setVoltage2   = value;
    sdk.sendClientAttribute("setVoltage2", setVoltage2);
    Serial.printf("[ATTR] limitVoltage2=%.2f → setVoltage2=%.2f\n", value, setVoltage2);
    return;
  }

  if (key == "limitVoltage3") {
    limitVoltage3 = value;
    setVoltage3   = value;
    sdk.sendClientAttribute("setVoltage3", setVoltage3);
    Serial.printf("[ATTR] limitVoltage3=%.2f → setVoltage3=%.2f\n", value, setVoltage3);
    return;
  }

  // ---- Generic fallthrough ----
  // Any unrecognised shared attribute: confirm back as client attribute.
  // Future widget keys require no code changes.
  sdk.sendClientAttribute(key, value);
}

// ============================================================
// CONNECTION CALLBACK
// ============================================================

void onConnect(bool connected) {
  Serial.println("[NET] Connected");

  // Re-request all shared attributes on every (re)connect.
  // Keeps device in sync with dashboard after MQTT reconnects.
  sdk.requestSharedAttributes();
}

void onDisconnect(bool connected) {
  Serial.println("[NET] Disconnected");
}

// ============================================================
// RPC CALLBACK
//
// payload = full command object: { method, params, requestId }
// Access params via payload["params"]["key"]
// ============================================================

void onRPC(const String& method, JsonObject payload) {

  Serial.printf("[RPC] method=%s\n", method.c_str());

  JsonObject params = payload["params"].as<JsonObject>();

  // ----------------------------------------------------------
  // PING
  // ----------------------------------------------------------
  if (method == "ping") {
    StaticJsonDocument<128> reply;
    reply["success"] = true;
    reply["message"] = "pong";
    reply["uptime"]  = millis();
    sdk.replyRPC(reply);
    return;
  }

  // ----------------------------------------------------------
  // GET STATUS
  // ----------------------------------------------------------
  if (method == "getStatus") {
    StaticJsonDocument<256> reply;
    reply["success"]       = true;
    reply["uptime"]        = millis();
    reply["freeHeap"]      = ESP.getFreeHeap();
    reply["wifiRSSI"]      = WiFi.RSSI();
    reply["channel1"]      = channelState[1] ? 1 : 0;
    reply["channel2"]      = channelState[2] ? 1 : 0;
    reply["channel3"]      = channelState[3] ? 1 : 0;
    reply["channel4"]      = channelState[4] ? 1 : 0;
    reply["volume"]      = volume;
    reply["setVoltage1"] = setVoltage1;
    reply["setVoltage2"] = setVoltage2;
    reply["setVoltage3"] = setVoltage3;
    sdk.replyRPC(reply);
    return;
  }

  // ----------------------------------------------------------
  // GET CONFIG
  // ----------------------------------------------------------
  if (method == "getConfig") {
    StaticJsonDocument<256> reply;
    reply["success"]    = true;
    reply["sdkVersion"] = 1.0;
    reply["freeHeap"]   = ESP.getFreeHeap();
    reply["flashSize"]  = ESP.getFlashChipSize();
    reply["chipRev"]    = ESP.getChipRevision();
    sdk.replyRPC(reply);
    return;
  }

  // ----------------------------------------------------------
  // GET DIAGNOSTICS
  // ----------------------------------------------------------
  if (method == "getDiagnostics") {
    StaticJsonDocument<256> reply;
    reply["success"]   = true;
    reply["uptime"]    = millis();
    reply["freeHeap"]  = ESP.getFreeHeap();
    reply["flashSize"] = ESP.getFlashChipSize();
    reply["chipRev"]   = ESP.getChipRevision();
    reply["wifiRSSI"]  = WiFi.RSSI();
    sdk.replyRPC(reply);
    return;
  }

  // ----------------------------------------------------------
  // SET VALUE — write an arbitrary client attribute
  // params: { key: "myKey", value: 42 }
  // ----------------------------------------------------------
  if (method == "setValue") {
    const char* key = params["key"] | "";
    float       val = params["value"] | 0.0f;

    if (strlen(key) == 0) {
      StaticJsonDocument<64> err;
      err["success"] = false;
      err["message"] = "key is required";
      sdk.replyRPC(err);
      return;
    }

    sdk.sendClientAttribute(String(key), val);

    StaticJsonDocument<128> reply;
    reply["success"] = true;
    reply["key"]     = key;
    reply["value"]   = val;
    sdk.replyRPC(reply);
    return;
  }

  // ----------------------------------------------------------
  // RELAY SET — directly drive a channel
  // params: { channel: 1, state: 1 }
  // ----------------------------------------------------------
  if (method == "relay_set") {
    int  ch    = params["channel"] | 1;
    bool state = (int)(params["state"] | 0) > 0;

    if (ch < 1 || ch > 4) {
      StaticJsonDocument<64> err;
      err["success"] = false;
      err["message"] = "channel must be 1-4";
      sdk.replyRPC(err);
      return;
    }

    applyChannel(ch, state ? 1.0f : 0.0f);

    StaticJsonDocument<128> reply;
    reply["success"] = true;
    reply["channel"] = ch;
    reply["state"]   = state ? 1 : 0;
    sdk.replyRPC(reply);
    return;
  }

  // ----------------------------------------------------------
  // RESET — turn off all channels, reset volume and voltage limits
  // ----------------------------------------------------------
  if (method == "reset") {
    for (int i = 1; i <= 4; i++) {
      applyChannel(i, 0.0f);
    }
    volume = 0.0f;
    sdk.sendClientAttribute("volume", 0.0f);

    limitVoltage1 = 0.0f; setVoltage1 = 245.0f;
    limitVoltage2 = 0.0f; setVoltage2 = 245.0f;
    limitVoltage3 = 0.0f; setVoltage3 = 245.0f;
    sdk.sendClientAttribute("setVoltage1", setVoltage1);
    sdk.sendClientAttribute("setVoltage2", setVoltage2);
    sdk.sendClientAttribute("setVoltage3", setVoltage3);

    StaticJsonDocument<64> reply;
    reply["success"] = true;
    reply["message"] = "Reset complete";
    sdk.replyRPC(reply);
    return;
  }

  // ----------------------------------------------------------
  // REBOOT
  // ----------------------------------------------------------
  if (method == "reboot") {
    StaticJsonDocument<64> reply;
    reply["success"] = true;
    reply["message"] = "Reboot accepted";
    sdk.replyRPC(reply);
    delay(1000);
    ESP.restart();
    return;
  }

  // ----------------------------------------------------------
  // TELEMETRY BURST — test tool
  // params: { count: 5 }
  // ----------------------------------------------------------
  if (method == "telemetry_burst") {
    int count = params["count"] | 5;
    if (count < 1)  count = 1;
    if (count > 20) count = 20;

    for (int i = 0; i < count; i++) {
      sdk.sendTelemetry("burstValue", (float)random(1, 100));
      delay(300);
    }

    StaticJsonDocument<64> reply;
    reply["success"] = true;
    reply["sent"]    = count;
    sdk.replyRPC(reply);
    return;
  }

  // ----------------------------------------------------------
  // UNKNOWN METHOD — never drop silently
  // ----------------------------------------------------------
  StaticJsonDocument<128> unknown;
  unknown["success"] = false;
  unknown["message"] = "Unknown method";
  unknown["method"]  = method;
  sdk.replyRPC(unknown);
}

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);
  delay(2000);

  // Init relay pins to OFF
  pinMode(RELAY_PIN_1, OUTPUT); digitalWrite(RELAY_PIN_1, LOW);
  pinMode(RELAY_PIN_2, OUTPUT); digitalWrite(RELAY_PIN_2, LOW);
  pinMode(RELAY_PIN_3, OUTPUT); digitalWrite(RELAY_PIN_3, LOW);
  pinMode(RELAY_PIN_4, OUTPUT); digitalWrite(RELAY_PIN_4, LOW);

  SDKConfig config;

  // ---- WiFi ----
  config.wifiSSID     = "71";
  config.wifiPassword = "90946062";

  // ---- MQTT ----
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;

  // ---- WSS ----
  config.wssPort = 8084;

  // ---- Device ----
  config.deviceToken = "cc00128e-10cb-4ec2-a135-401612fc7935";

  // ---- Transport ----
  config.enableWS   = true;
  config.enableMQTT = true;

  // ---- TLS ----
  config.allowInsecureTLS = false;
  config.rootCA           = AUTOCONNECTO_ROOT_CA;

  // ---- Debug ----
  config.enableSerialLogs = true;

  // ---- Callbacks ----
  sdk.onAttributeUpdate(onAttributeUpdate);
  sdk.onConnect(onConnect);
  sdk.onDisconnect(onDisconnect);
  sdk.onRPC(onRPC);

  sdk.begin(config);

  // Request ALL shared attributes at startup.
  // Triggers onAttributeUpdate for each stored key →
  // device applies values → confirms via client attributes.
  // This is the power-cycle sync mechanism.
  sdk.requestSharedAttributes();

  Serial.println("[SDK] Started");
}

// ============================================================
// LOOP
// ============================================================

unsigned long lastTelemetry   = 0;
unsigned long lastStatusPrint = 0;

void loop() {

  // CRITICAL: sdk.loop() must run frequently.
  // Never use delay() > ~50ms — starves MQTT keepalive.
  sdk.loop();

  unsigned long now = millis();

  // ---- Periodic transport status (every 30s) ----
  if (now - lastStatusPrint > 30000) {
    lastStatusPrint = now;
    Serial.printf(
      "[STATUS] transport=%s wss=%d mqtts=%d heap=%u\n",
      sdk.activeTransport().c_str(),
      sdk.isUsingWSS(),
      sdk.isUsingMQTTS(),
      ESP.getFreeHeap()
    );
  }

  // ---- Periodic telemetry (every 10s) ----
  if (now - lastTelemetry > 10000) {
    lastTelemetry = now;

    // Single key
    sdk.sendTelemetry("temperature", (float)random(20, 35));

    // Up to 3 keys in one call (SDK limit for variadic overload)
    sdk.sendTelemetry(
      "humidity", (float)random(40, 80),
      "current",  (float)random(1, 10),
      "power",    (float)random(100, 500)
    );

    // 4+ keys must use JsonDocument
    StaticJsonDocument<256> voltages;
    voltages["voltage1"] = (float)random(220, 280);
    voltages["voltage2"] = (float)random(220, 280);
    voltages["voltage3"] = (float)random(220, 280);
    sdk.sendTelemetry(voltages);

    // Device health client attributes
    StaticJsonDocument<256> attrs;
    attrs["freeHeap"]   = ESP.getFreeHeap();
    attrs["wifiRSSI"]   = WiFi.RSSI();
    attrs["uptime"]     = millis() / 1000;
    attrs["sdkVersion"] = 1.0;
    sdk.sendClientAttributes(attrs);
  }

  // Short yield — keeps MQTT keepalive healthy
  delay(10);
}
