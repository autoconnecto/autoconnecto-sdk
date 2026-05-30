// =============================================================
// Machine_Runtime_NFC_mqtt — Autoconnecto SDK (Rev 2)
//
// Contract:  sdk/MACHINE_RUNTIME_NFC.md
// Hardware:  sdk/MACHINE_RUNTIME_HARDWARE_BOM.md § Rev 2.0
// Reader:    Mifra PN532 MT0359 — I2C mode (GPIO 21/22)
// Cards:     MIFARE Classic 1K — employee_id + display_name on card
//
// Libraries (Arduino Library Manager):
//   - Adafruit PN532
//   - Adafruit BusIO
//
// Enrollment: flash Machine_Runtime_NFC_enroll.ino first to write cards
// =============================================================

#include <Wire.h>
#include <Adafruit_PN532.h>
#include <AutoconnectoSDK.h>

AutoconnectoSDK sdk;

// --- Telemetry / attribute keys ---
const char* KEY_CURRENT = "machine_current_a";
const char* KEY_SENSOR_OK = "machine_sensor_ok";
const char* KEY_OPERATOR_ID = "machine_operator_id";
const char* KEY_OPERATOR_NAME = "machine_operator_name";
const char* KEY_SESSION_ACTIVE = "machine_session_active";
const char* ATTR_ALLOW_RUN = "machine_allow_run";
const char* ATTR_TOOL_REMAINING = "machine_tool_remaining";
const char* ATTR_TOOL_LIMIT = "machine_tool_limit";
const char* ATTR_TOOL_USED = "machine_tool_cycles_used";

// --- PZEM (unchanged Rev 1) ---
#define PZEM_UART_RX 16
#define PZEM_UART_TX 17
#define PZEM_BAUD 9600
#define PZEM_SLAVE_ADDR 0xF8
#define PZEM_DEMO_FALLBACK 1

// --- PN532 I2C (Rev 2 frozen) ---
#define PN532_IRQ 4
#define PN532_RESET 5
#define I2C_SDA 21
#define I2C_SCL 22

#define PIN_SSR_ALLOW 26

#define SHARED_SYNC_MS 60000UL
#define CLIENT_PUSH_MS 30000UL
#define TELEMETRY_MS 10000UL
#define NFC_DEBOUNCE_MS 2500UL

// MIFARE Classic 1K — sector 1 data blocks (see MACHINE_RUNTIME_NFC.md)
#define MIFARE_SECTOR 1
#define MIFARE_BLOCK_MAGIC 4
#define MIFARE_BLOCK_EMP_ID 5
#define MIFARE_BLOCK_NAME 6
static const char CARD_MAGIC[] = "ACMRUNv1";

static uint8_t g_mifareKeyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

HardwareSerial PzemSerial(2);
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

static bool allowRun = true;
static bool sessionActive = false;
static String operatorId = "";
static String operatorName = "";
static uint8_t lastUid[7] = {0};
static uint8_t lastUidLen = 0;
static unsigned long lastTapMs = 0;

static int toolRemaining = -1;
static int toolLimit = -1;
static int toolUsed = -1;

// --- SSR ---
static void applySsrOutput() {
  digitalWrite(PIN_SSR_ALLOW, (allowRun && sessionActive) ? HIGH : LOW);
}

// --- Platform sync ---
static void pushClientMirror(bool pushOperatorTelemetry = false) {
  StaticJsonDocument<320> attrs;
  attrs[ATTR_ALLOW_RUN] = allowRun;
  attrs[KEY_SESSION_ACTIVE] = sessionActive;
  if (operatorId.length()) attrs[KEY_OPERATOR_ID] = operatorId;
  if (operatorName.length()) attrs[KEY_OPERATOR_NAME] = operatorName;
  if (toolRemaining >= 0) attrs[ATTR_TOOL_REMAINING] = toolRemaining;
  if (toolLimit >= 0) attrs[ATTR_TOOL_LIMIT] = toolLimit;
  if (toolUsed >= 0) attrs[ATTR_TOOL_USED] = toolUsed;
  sdk.sendClientAttributes(attrs);

  if (pushOperatorTelemetry) {
    StaticJsonDocument<256> tel;
    tel[KEY_OPERATOR_ID] = operatorId;
    tel[KEY_OPERATOR_NAME] = operatorName;
    tel[KEY_SESSION_ACTIVE] = sessionActive;
    sdk.sendTelemetry(tel);
  }
}

static void requestPlatformSync(const char* reason) {
  Serial.print("[SYNC] ");
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
      lastUidLen = 0;
    }
    applySsrOutput();
    return;
  }
  if (key == ATTR_TOOL_REMAINING) {
    toolRemaining = (int)value;
    if (toolRemaining <= 0) allowRun = false;
    applySsrOutput();
    return;
  }
  if (key == ATTR_TOOL_LIMIT) toolLimit = (int)value;
  if (key == ATTR_TOOL_USED) toolUsed = (int)value;
}

static void onConnect(bool connected) {
  if (connected) {
    requestPlatformSync("mqtt_connected");
    pushClientMirror(false);
  }
}

// --- NFC helpers ---
static String blockToAscii(const uint8_t* block, size_t maxLen) {
  char buf[33];
  size_t n = maxLen < 32 ? maxLen : 32;
  memcpy(buf, block, n);
  buf[n] = 0;
  for (int i = (int)n - 1; i >= 0; i--) {
    if (buf[i] == ' ' || buf[i] == '\0') buf[i] = 0;
    else break;
  }
  String s(buf);
  s.trim();
  return s;
}

static bool uidEqual(const uint8_t* a, uint8_t alen, const uint8_t* b, uint8_t blen) {
  if (alen != blen || alen == 0) return false;
  return memcmp(a, b, alen) == 0;
}

static void copyUid(const uint8_t* uid, uint8_t len) {
  lastUidLen = len > 7 ? 7 : len;
  memset(lastUid, 0, sizeof(lastUid));
  memcpy(lastUid, uid, lastUidLen);
}

static bool authenticateSector1(const uint8_t* uid, uint8_t uidLen) {
  return nfc.mifareclassic_AuthenticateBlock(
    uid, MIFARE_BLOCK_MAGIC, MIFARE_SECTOR, MIFARE_CMD_AUTH_A, g_mifareKeyA);
}

static bool readWorkerFromCard(const uint8_t* uid, uint8_t uidLen, String& empId, String& displayName) {
  uint8_t block[16];

  if (!authenticateSector1(uid, uidLen)) {
    Serial.println("[NFC] auth sector 1 failed (wrong key or card type)");
    return false;
  }

  if (!nfc.mifareclassic_ReadDataBlock(MIFARE_BLOCK_MAGIC, block)) {
    Serial.println("[NFC] read magic block failed");
    return false;
  }
  if (memcmp(block, CARD_MAGIC, 8) != 0) {
    Serial.print("[NFC] bad magic, expected ");
    Serial.println(CARD_MAGIC);
    return false;
  }

  if (!nfc.mifareclassic_ReadDataBlock(MIFARE_BLOCK_EMP_ID, block)) {
    Serial.println("[NFC] read employee_id block failed");
    return false;
  }
  empId = blockToAscii(block, 16);
  if (!empId.length()) {
    Serial.println("[NFC] empty employee_id on card");
    return false;
  }

  if (!nfc.mifareclassic_ReadDataBlock(MIFARE_BLOCK_NAME, block)) {
    Serial.println("[NFC] read display_name block failed");
    return false;
  }
  displayName = blockToAscii(block, 32);
  if (!displayName.length()) displayName = empId;

  return true;
}

static void handleWorkerTap(const uint8_t* uid, uint8_t uidLen, const String& empId, const String& displayName) {
  if (!allowRun) {
    Serial.println("[NFC] tool life blocked — tap ignored");
    return;
  }

  const bool sameCard = uidEqual(uid, uidLen, lastUid, lastUidLen);
  const unsigned long now = millis();
  if (sameCard && (now - lastTapMs) < NFC_DEBOUNCE_MS) return;
  lastTapMs = now;
  copyUid(uid, uidLen);

  if (!sessionActive || !sameCard) {
    sessionActive = true;
    operatorId = empId;
    operatorName = displayName;
    Serial.print("[NFC] IN  id=");
    Serial.print(operatorId);
    Serial.print(" name=");
    Serial.println(operatorName);
  } else {
    sessionActive = false;
    operatorId = "";
    operatorName = "";
    Serial.println("[NFC] OUT (same card tap)");
  }

  applySsrOutput();
  pushClientMirror(true);
}

static void pollNfc() {
  uint8_t uid[7] = {0};
  uint8_t uidLen = 0;

  if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen, 80)) {
    return;
  }

  String empId;
  String displayName;
  if (!readWorkerFromCard(uid, uidLen, empId, displayName)) {
    Serial.println("[NFC] card not enrolled — run enroll sketch first");
    return;
  }

  handleWorkerTap(uid, uidLen, empId, displayName);
}

static bool initPn532() {
  Wire.begin(I2C_SDA, I2C_SCL);
  nfc.begin();

  uint32_t version = nfc.getFirmwareVersion();
  if (!version) {
    Serial.println("[NFC] PN532 not found — check I2C wiring and module mode switch (I2C)");
    return false;
  }
  Serial.print("[NFC] PN532 firmware 0x");
  Serial.println(version, HEX);
  nfc.SAMConfig();
  return true;
}

// --- PZEM (same as Rev 1) ---
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
    if (PzemSerial.available()) resp[idx++] = (uint8_t)PzemSerial.read();
  }
  if (idx < 5 || resp[0] != slave || resp[1] != 0x04) return false;
  const uint8_t byteCount = resp[2];
  if (idx < (size_t)(3 + byteCount + 2)) return false;
  const uint16_t rxCrc = (uint16_t)resp[3 + byteCount] | ((uint16_t)resp[4 + byteCount] << 8);
  if (modbusCRC(resp, 3 + byteCount) != rxCrc) return false;
  for (uint16_t i = 0; i < count; i++) {
    out[i] = ((uint16_t)resp[3 + i * 2] << 8) | resp[4 + i * 2];
  }
  return true;
}

static float readPZEMCurrentAmps() {
  uint16_t regs[2] = {0, 0};
  if (!modbusReadInputRegs(PZEM_SLAVE_ADDR, 0x0001, 2, regs)) return NAN;
  return (((uint32_t)regs[0] << 16) | regs[1]) / 1000.0f;
}

static float readDemoCurrentAmps() {
  const unsigned long phase = (millis() / 40000UL) % 3UL;
  if (phase == 0) return 0.2f;
  if (phase == 1) return 4.0f;
  return 22.0f;
}

static float readCurrentAmps(bool* sensorOk) {
  float amps = readPZEMCurrentAmps();
  if (!isnan(amps) && amps >= 0.0f && amps < 120.0f) {
    *sensorOk = true;
    return amps;
  }
#if PZEM_DEMO_FALLBACK
  *sensorOk = false;
  return readDemoCurrentAmps();
#else
  *sensorOk = false;
  return 0.0f;
#endif
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(PIN_SSR_ALLOW, OUTPUT);
  digitalWrite(PIN_SSR_ALLOW, LOW);

  PzemSerial.begin(PZEM_BAUD, SERIAL_8N1, PZEM_UART_RX, PZEM_UART_TX);
  delay(100);

  if (!initPn532()) {
    Serial.println("[NFC] HALT — fix PN532 before deployment");
  }

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

  Serial.println("[SDK] Machine_Runtime_NFC — PN532 + PZEM + SSR");
}

unsigned long lastTelemetryMs = 0;
unsigned long lastSharedSyncMs = 0;
unsigned long lastClientPushMs = 0;

void loop() {
  sdk.loop();
  pollNfc();

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
    const float amps = readCurrentAmps(&sensorOk);

    StaticJsonDocument<320> tel;
    tel[KEY_CURRENT] = amps;
    tel[KEY_SENSOR_OK] = sensorOk;
    tel[KEY_SESSION_ACTIVE] = sessionActive;
    if (sessionActive) {
      tel[KEY_OPERATOR_ID] = operatorId;
      tel[KEY_OPERATOR_NAME] = operatorName;
    }
    sdk.sendTelemetry(tel);

    Serial.print("[TEL] I=");
    Serial.print(amps, 2);
    Serial.print(" op=");
    Serial.print(sessionActive ? operatorId : "-");
    Serial.print(" session=");
    Serial.println(sessionActive ? "1" : "0");
  }

  delay(20);
}
