// =============================================================
// SwitchControl_mqtt — Autoconnecto SDK example
//
// Pair sketch: examples/SwitchControl_http/SwitchControl_http.ino
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

// TLS: AUTOCONNECTO_ROOT_CA from AutoconnectoIsrgRoots.h (via AutoconnectoSDK.h)

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

  Serial.println("[SDK] SwitchControl_mqtt started");
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
