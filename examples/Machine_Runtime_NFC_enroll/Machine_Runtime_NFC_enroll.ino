// =============================================================
// Machine_Runtime_NFC_enroll — write worker id + name to MIFARE card
//
// Flash on desk ESP32 + PN532 (same wiring as production).
// Serial Monitor 115200:
//   w EMP1042 Rajesh Kumar     — write card (hold card on reader)
//   r                        — read back
//
// Card layout: sdk/MACHINE_RUNTIME_NFC.md
// =============================================================

#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ 4
#define PN532_RESET 5
#define I2C_SDA 21
#define I2C_SCL 22

#define MIFARE_SECTOR 1
#define MIFARE_BLOCK_MAGIC 4
#define MIFARE_BLOCK_EMP_ID 5
#define MIFARE_BLOCK_NAME 6
static const char CARD_MAGIC[] = "ACMRUNv1";

static uint8_t g_keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

static void padBlock(const char* ascii, uint8_t* block, size_t maxLen) {
  memset(block, 0, 16);
  size_t n = strlen(ascii);
  if (n > maxLen) n = maxLen;
  memcpy(block, ascii, n);
}

static bool waitForCard(uint8_t* uid, uint8_t* uidLen) {
  Serial.println("Place MIFARE card on PN532...");
  for (int i = 0; i < 50; i++) {
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, uidLen, 100)) {
      return true;
    }
    delay(200);
  }
  return false;
}

static bool writeWorkerCard(const String& empId, const String& displayName) {
  uint8_t uid[7] = {0};
  uint8_t uidLen = 0;

  if (!waitForCard(uid, &uidLen)) {
    Serial.println("No card detected");
    return false;
  }

  if (!nfc.mifareclassic_AuthenticateBlock(uid, MIFARE_BLOCK_MAGIC, MIFARE_SECTOR, MIFARE_CMD_AUTH_A, g_keyA)) {
    Serial.println("Auth failed — use MIFARE Classic 1K cards");
    return false;
  }

  uint8_t block[16];
  padBlock(CARD_MAGIC, block, 8);
  if (!nfc.mifareclassic_WriteDataBlock(MIFARE_BLOCK_MAGIC, block)) {
    Serial.println("Write magic failed");
    return false;
  }

  padBlock(empId.c_str(), block, 16);
  if (!nfc.mifareclassic_WriteDataBlock(MIFARE_BLOCK_EMP_ID, block)) {
    Serial.println("Write employee_id failed");
    return false;
  }

  padBlock(displayName.c_str(), block, 32);
  if (!nfc.mifareclassic_WriteDataBlock(MIFARE_BLOCK_NAME, block)) {
    Serial.println("Write display_name failed");
    return false;
  }

  Serial.println("OK — card written:");
  Serial.print("  id:   ");
  Serial.println(empId);
  Serial.print("  name: ");
  Serial.println(displayName);
  return true;
}

static bool readWorkerCard() {
  uint8_t uid[7] = {0};
  uint8_t uidLen = 0;

  if (!waitForCard(uid, &uidLen)) return false;

  if (!nfc.mifareclassic_AuthenticateBlock(uid, MIFARE_BLOCK_MAGIC, MIFARE_SECTOR, MIFARE_CMD_AUTH_A, g_keyA)) {
    Serial.println("Auth failed");
    return false;
  }

  uint8_t block[16];
  nfc.mifareclassic_ReadDataBlock(MIFARE_BLOCK_MAGIC, block);
  block[8] = 0;
  Serial.print("magic: ");
  Serial.println((char*)block);

  nfc.mifareclassic_ReadDataBlock(MIFARE_BLOCK_EMP_ID, block);
  block[15] = 0;
  Serial.print("id:    ");
  Serial.println((char*)block);

  nfc.mifareclassic_ReadDataBlock(MIFARE_BLOCK_NAME, block);
  block[15] = 0;
  Serial.print("name:  ");
  Serial.println((char*)block);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(I2C_SDA, I2C_SCL);
  nfc.begin();

  if (!nfc.getFirmwareVersion()) {
    Serial.println("PN532 not found — I2C mode, GPIO 21/22");
    while (1) delay(1000);
  }
  nfc.SAMConfig();

  Serial.println("=== Machine Runtime NFC enrollment ===");
  Serial.println("  w <employee_id> <display name>");
  Serial.println("  r  — read card");
  Serial.println("Example: w worker1 Rajesh Kumar");
}

void loop() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (!line.length()) return;

  if (line.startsWith("w ")) {
    line = line.substring(2);
    line.trim();
    int sp = line.indexOf(' ');
    if (sp < 1) {
      Serial.println("Usage: w worker1 Rajesh Kumar");
      return;
    }
    writeWorkerCard(line.substring(0, sp), line.substring(sp + 1));
    return;
  }

  if (line == "r") {
    readWorkerCard();
    return;
  }

  Serial.println("Unknown command");
}
