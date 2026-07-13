// =============================================================
// Machine_Runtime_mqtt — Autoconnecto SDK example
//
// Dashboard: Machine Fleet Runtime (machineFleetRuntime)
// Contract:  sdk/MACHINE_RUNTIME.md
// Hardware:  sdk/MACHINE_RUNTIME_HARDWARE_BOM.md
//
// TELEMETRY:
//   machine_current_a, machine_voltage_v, machine_power_w, machine_sensor_ok
//   machine_operator_id, machine_operator_name, machine_session_active
//
// SHARED (platform → device, synced on boot + reconnect + every 60s):
//   machine_allow_run, machine_tool_remaining, machine_tool_limit,
//   machine_tool_cycles_used
//
// CLIENT (device → platform, every 30s + after RFID):
//   session + tool mirror for power-cycle recovery
//
// HARDWARE (Rev 1.0):
//   PZEM UART2: RX=16 TX=17
//   RFID UART1: RX=18 TX=19 (125 kHz EM4100 reader, 9600 baud)
//   Fotek SSR IN: GPIO 26 (HIGH = allow contactor enable)
// =============================================================

#include <AutoconnectoSDK.h>

AutoconnectoSDK sdk;

const char* KEY_CURRENT = "machine_current_a";
const char* KEY_VOLTAGE = "machine_voltage_v";
const char* KEY_POWER = "machine_power_w";
const char* KEY_SENSOR_OK = "machine_sensor_ok";
const char* KEY_OPERATOR_ID = "machine_operator_id";
const char* KEY_OPERATOR_NAME = "machine_operator_name";
const char* KEY_SESSION_ACTIVE = "machine_session_active";
const char* ATTR_ALLOW_RUN = "machine_allow_run";
const char* ATTR_TOOL_REMAINING = "machine_tool_remaining";
const char* ATTR_TOOL_LIMIT = "machine_tool_limit";
const char* ATTR_TOOL_USED = "machine_tool_cycles_used";

#define PZEM_UART_RX 16
#define PZEM_UART_TX 17
#define RFID_UART_RX 18
#define RFID_UART_TX 19
#define PIN_SSR_ALLOW 26

#define PZEM_BAUD 9600
#define RFID_BAUD 9600
#define PZEM_SLAVE_ADDR 0xF8
#define PZEM_DEMO_FALLBACK 1
#define RFID_ENABLED 1

struct PzemReading {
  float voltageV;
  float currentA;
  float powerW;
};

#define SHARED_SYNC_MS 60000UL
#define CLIENT_PUSH_MS 30000UL
/** PZEM: read and MQTT publish every 2s */
#define TELEMETRY_MS 2000UL

HardwareSerial PzemSerial(2);
HardwareSerial RfidSerial(1);

static bool allowRun = true;
static bool sessionActive = false;
static String operatorId = "";
static String operatorName = "";
static String lastCardUid = "";
static int toolRemaining = -1;
static int toolLimit = -1;
static int toolUsed = -1;

static void applySsrOutput() {
  const bool energize = allowRun && sessionActive;
  digitalWrite(PIN_SSR_ALLOW, energize ? HIGH : LOW);
}

static void pushClientMirror(bool forceTelemetryKeys = false) {
  StaticJsonDocument<256> attrs;
  attrs[ATTR_ALLOW_RUN] = allowRun;
  attrs[KEY_SESSION_ACTIVE] = sessionActive;
  if (operatorId.length()) attrs[KEY_OPERATOR_ID] = operatorId;
  if (operatorName.length()) attrs[KEY_OPERATOR_NAME] = operatorName;
  if (toolRemaining >= 0) attrs[ATTR_TOOL_REMAINING] = toolRemaining;
  if (toolLimit >= 0) attrs[ATTR_TOOL_LIMIT] = toolLimit;
  if (toolUsed >= 0) attrs[ATTR_TOOL_USED] = toolUsed;
  sdk.sendClientAttributes(attrs);

  if (forceTelemetryKeys) {
    StaticJsonDocument<192> tel;
    tel[KEY_OPERATOR_ID] = operatorId;
    tel[KEY_OPERATOR_NAME] = operatorName;
    tel[KEY_SESSION_ACTIVE] = sessionActive;
    sdk.sendTelemetry(tel);
  }
}

static void requestPlatformSync(const char* reason) {
  Serial.print("[SYNC] request shared — ");
  Serial.println(reason);
  sdk.requestSharedAttributes();
}

static void onSharedAttribute(const String& key, float value) {
  if (key == ATTR_ALLOW_RUN) {
    allowRun = value >= 0.5f;
    if (!allowRun) {
      sessionActive = false;
      operatorId = "";
      operatorName = "";
      lastCardUid = "";
    }
    applySsrOutput();
    Serial.print("[ATTR] ");
    Serial.print(ATTR_ALLOW_RUN);
    Serial.print("=");
    Serial.println(allowRun ? "1" : "0");
    return;
  }

  if (key == ATTR_TOOL_REMAINING) {
    toolRemaining = (int)value;
    if (toolRemaining <= 0) allowRun = false;
    applySsrOutput();
    Serial.print("[ATTR] tool remaining=");
    Serial.println(toolRemaining);
    return;
  }

  if (key == ATTR_TOOL_LIMIT) {
    toolLimit = (int)value;
    return;
  }

  if (key == ATTR_TOOL_USED) {
    toolUsed = (int)value;
    return;
  }
}

static void onConnect(bool connected) {
  if (connected) {
    requestPlatformSync("mqtt_connected");
    pushClientMirror(false);
  } else {
    Serial.println("[SYNC] mqtt disconnected");
  }
}

#if RFID_ENABLED
static bool parseRdm6300Frame(const uint8_t* buf, size_t len, String& uidOut) {
  if (len < 14) return false;
  for (size_t i = 0; i + 13 < len; i++) {
    if (buf[i] != 0x02) continue;
    if (buf[i + 13] != 0x03) continue;
    char hex[11];
    for (int j = 0; j < 10; j++) {
      const uint8_t n = buf[i + 1 + j];
      if (n < 0x30) return false;
      hex[j] = (char)n;
    }
    hex[10] = 0;
    uidOut = String(hex);
    return true;
  }
  return false;
}

static void handleCardUid(const String& uid) {
  const String normalized = uid;
  if (!normalized.length()) return;

  if (!allowRun) {
    Serial.println("[RFID] Tool life expired — tap ignored");
    return;
  }

  if (!sessionActive || lastCardUid != normalized) {
    sessionActive = true;
    lastCardUid = normalized;
    operatorId = normalized;
    operatorName = normalized;
    Serial.print("[RFID] Session START ");
    Serial.println(normalized);
  } else {
    sessionActive = false;
    operatorId = "";
    operatorName = "";
    Serial.print("[RFID] Session END ");
    Serial.println(normalized);
  }

  applySsrOutput();
  pushClientMirror(true);
}

static void pollRfid() {
  static uint8_t buf[64];
  static size_t len = 0;
  while (RfidSerial.available()) {
    const uint8_t b = (uint8_t)RfidSerial.read();
    if (len < sizeof(buf)) buf[len++] = b;
    String uid;
    if (parseRdm6300Frame(buf, len, uid)) {
      handleCardUid(uid);
      len = 0;
    }
    if (len >= sizeof(buf)) len = 0;
  }
}
#endif

static uint16_t modbusCRC(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 1) crc = (crc >> 1) ^ 0xA001;
      else crc >>= 1;
    }
  }
  return crc;
}

static bool modbusReadInputRegs(uint8_t slave, uint16_t startReg, uint16_t count, uint16_t* out) {
  if (!count || count > 32) return false;

  uint8_t req[8];
  req[0] = slave;
  req[1] = 0x04;
  req[2] = (uint8_t)(startReg >> 8);
  req[3] = (uint8_t)(startReg & 0xFF);
  req[4] = (uint8_t)(count >> 8);
  req[5] = (uint8_t)(count & 0xFF);
  const uint16_t crc = modbusCRC(req, 6);
  req[6] = (uint8_t)(crc & 0xFF);
  req[7] = (uint8_t)(crc >> 8);

  while (PzemSerial.available()) PzemSerial.read();

  PzemSerial.write(req, 8);
  PzemSerial.flush();

  const unsigned long deadline = millis() + 500;
  size_t idx = 0;
  uint8_t resp[128];
  const size_t expected = 5 + count * 2;

  while (millis() < deadline && idx < expected && idx < sizeof(resp)) {
    if (PzemSerial.available()) {
      resp[idx++] = (uint8_t)PzemSerial.read();
    }
  }

  if (idx < 5) return false;
  if (resp[0] != slave || resp[1] != 0x04) return false;

  const uint8_t byteCount = resp[2];
  if (idx < (size_t)(3 + byteCount + 2)) return false;

  const uint16_t rxCrc = (uint16_t)resp[3 + byteCount] | ((uint16_t)resp[4 + byteCount] << 8);
  if (modbusCRC(resp, 3 + byteCount) != rxCrc) return false;

  for (uint16_t i = 0; i < count; i++) {
    out[i] = ((uint16_t)resp[3 + i * 2] << 8) | resp[4 + i * 2];
  }
  return true;
}

static bool readPZEM(PzemReading& out) {
  uint16_t regs[5] = {0, 0, 0, 0, 0};
  if (!modbusReadInputRegs(PZEM_SLAVE_ADDR, 0x0000, 5, regs)) {
    return false;
  }
  out.voltageV = regs[0] / 10.0f;
  const uint32_t currentRaw = ((uint32_t)regs[2] << 16) | regs[1];
  out.currentA = currentRaw / 1000.0f;
  const uint32_t powerRaw = ((uint32_t)regs[4] << 16) | regs[3];
  out.powerW = powerRaw / 10.0f;
  return true;
}

static bool pzemReadingValid(const PzemReading& r) {
  return r.voltageV >= 0.0f && r.voltageV <= 320.0f &&
         r.currentA >= 0.0f && r.currentA < 120.0f &&
         r.powerW >= 0.0f && r.powerW < 35000.0f;
}

static float readDemoCurrentAmps() {
  const unsigned long phase = (millis() / 40000UL) % 3UL;
  if (phase == 0) return 0.2f;
  if (phase == 1) return 4.0f;
  return 22.0f;
}

static float readCurrentAmps(bool* sensorOk, float* voltageV = nullptr, float* powerW = nullptr) {
  PzemReading pzem;
  if (readPZEM(pzem) && pzemReadingValid(pzem)) {
    *sensorOk = true;
    if (voltageV) *voltageV = pzem.voltageV;
    if (powerW) *powerW = pzem.powerW;
    return pzem.currentA;
  }

#if PZEM_DEMO_FALLBACK
  *sensorOk = false;
  if (voltageV) *voltageV = 230.0f;
  if (powerW) {
    const float demo = readDemoCurrentAmps();
    *powerW = demo * 230.0f;
  }
  return readDemoCurrentAmps();
#else
  *sensorOk = false;
  if (voltageV) *voltageV = 0.0f;
  if (powerW) *powerW = 0.0f;
  return 0.0f;
#endif
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(PIN_SSR_ALLOW, OUTPUT);
  digitalWrite(PIN_SSR_ALLOW, LOW);

  PzemSerial.begin(PZEM_BAUD, SERIAL_8N1, PZEM_UART_RX, PZEM_UART_TX);
#if RFID_ENABLED
  RfidSerial.begin(RFID_BAUD, SERIAL_8N1, RFID_UART_RX, RFID_UART_TX);
#endif
  delay(100);

  SDKConfig config;
  config.wifiSSID = "YOUR_WIFI_SSID";
  config.wifiPassword = "YOUR_WIFI_PASSWORD";
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.wssPort = 8084;
  config.deviceToken = "YOUR_DEVICE_TOKEN";
  config.enableWS = true;
  config.enableMQTT = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = true;

  sdk.begin(config);
  sdk.onAttributeUpdate(onSharedAttribute);
  sdk.onConnect(onConnect);

  requestPlatformSync("boot");
  applySsrOutput();

  Serial.println("[SDK] Machine_Runtime — PZEM + RFID + SSR + periodic sync");
}

unsigned long lastTelemetryMs = 0;
unsigned long lastSharedSyncMs = 0;
unsigned long lastClientPushMs = 0;

void loop() {
  sdk.loop();

#if RFID_ENABLED
  pollRfid();
#endif

  const unsigned long nowMs = millis();

  if (nowMs - lastSharedSyncMs >= SHARED_SYNC_MS) {
    lastSharedSyncMs = nowMs;
    requestPlatformSync("periodic");
  }

  if (nowMs - lastClientPushMs >= CLIENT_PUSH_MS) {
    lastClientPushMs = nowMs;
    pushClientMirror(false);
  }

  if (nowMs - lastTelemetryMs >= TELEMETRY_MS) {
    lastTelemetryMs = nowMs;

    bool sensorOk = true;
    float voltageV = 0.0f;
    float powerW = 0.0f;
    const float amps = readCurrentAmps(&sensorOk, &voltageV, &powerW);

    StaticJsonDocument<320> tel;
    tel[KEY_CURRENT] = amps;
    tel[KEY_VOLTAGE] = voltageV;
    tel[KEY_POWER] = powerW;
    tel[KEY_SENSOR_OK] = sensorOk;
    if (sessionActive) {
      tel[KEY_OPERATOR_ID] = operatorId;
      tel[KEY_OPERATOR_NAME] = operatorName;
    }
    tel[KEY_SESSION_ACTIVE] = sessionActive;
    sdk.sendTelemetry(tel);

    Serial.print("[TEL] V=");
    Serial.print(voltageV, 1);
    Serial.print("V I=");
    Serial.print(amps, 2);
    Serial.print("A P=");
    Serial.print(powerW, 0);
    Serial.print("W session=");
    Serial.print(sessionActive ? "1" : "0");
    Serial.print(" allow=");
    Serial.println(allowRun ? "1" : "0");
  }

  delay(10);
}
