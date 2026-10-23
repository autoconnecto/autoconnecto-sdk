// =============================================================
// OtaFirmwareUpdate_mqtt — Autoconnecto SDK example (FOTA)
//
// PURPOSE
//   Download firmware when the platform assigns a FIRMWARE package
//   (shared fw_* attributes) and flash via esp_ota.
//
// SETUP
//   1. Flash this sketch with your WiFi + device token.
//   2. In the app: OTA → Firmware → Upload .bin → Assign to this device.
//   3. Watch Serial for [OTA] lines and fw_state on the device.
//
// IMPORTANT
//   Register onAttribute* / onConnect callbacks BEFORE sdk.begin().
//   begin() copies handlers into the MQTT transport. (SDK 1.7.57+
//   also re-applies handlers registered after begin.)
// =============================================================

#include <AutoconnectoSDK.h>
#include <OtaUpdate.h>
#include <ArduinoJson.h>
#include "secrets.h"

AutoconnectoSDK sdk;
AutoconnectoOta ota;

void setup() {
  Serial.begin(115200);
  delay(2000);

  SDKConfig config;
  config.wifiSSID = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;
  config.deviceToken = DEVICE_TOKEN;
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.mqttUseTls = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.sharedAttributeKeys =
    "fw_title,fw_version,fw_size,fw_checksum,fw_checksum_algorithm";
  config.enableSerialLogs = true;

  OtaConfig otaCfg;
  otaCfg.apiHost = "api.autoconnecto.in";
  otaCfg.deviceToken = config.deviceToken;
  otaCfg.rootCA = AUTOCONNECTO_ROOT_CA;
  otaCfg.autoReboot = true;

  ota.begin(otaCfg, [](const char* key, const char* value) {
    JsonDocument doc;
    doc[key] = value;
    return sdk.sendClientAttributes(doc);
  });
  ota.onProgressReport([](const char* stateKey, const char* state, const char* title, const char* version) {
    JsonDocument doc;
    doc[stateKey] = state;
    doc["fw_title"] = title;
    doc["fw_version"] = version;
    return sdk.sendClientAttributes(doc);
  });

  // Callbacks MUST be registered before begin() so the transport receives them.
  sdk.onAttributeStringUpdate([](const String& key, const String& value) {
    ota.onSharedAttribute(key, value);
  });
  sdk.onAttributeUpdate([](const String& key, float value) {
    ota.onSharedAttribute(key, value);
  });
  sdk.onConnect([](bool ok) {
    if (ok) {
      sdk.requestSharedAttributes(
        "fw_title,fw_version,fw_size,fw_checksum,fw_checksum_algorithm"
      );
    }
  });

  sdk.begin(config);
}

void loop() {
  sdk.loop();
  const bool wasBusy = ota.isBusy();
  ota.loop();
  if (wasBusy && !ota.isBusy()) {
    sdk.requestSharedAttributes(
      "fw_title,fw_version,fw_size,fw_checksum,fw_checksum_algorithm"
    );
  }
}
