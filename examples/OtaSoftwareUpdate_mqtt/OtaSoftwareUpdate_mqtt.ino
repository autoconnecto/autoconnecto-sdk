// =============================================================
// OtaSoftwareUpdate_mqtt — Autoconnecto SDK example (SOTA)
//
// PURPOSE
//   Download a SOFTWARE package when the platform assigns one
//   (shared sw_* attributes) to LittleFS, then notify your sketch.
//   Does not flash firmware and does not reboot.
//
// SETUP
//   1. Flash this sketch with your WiFi + device token.
//   2. In the app: OTA → Software → Upload package → Assign.
//   3. Serial prints the LittleFS path when the file is ready.
//
// IMPORTANT
//   Register onAttribute* / onConnect callbacks BEFORE sdk.begin().
// =============================================================

#include <AutoconnectoSDK.h>
#include <OtaUpdate.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
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
    "sw_title,sw_version,sw_size,sw_checksum,sw_checksum_algorithm";
  config.enableSerialLogs = true;

  OtaConfig otaCfg;
  otaCfg.apiHost = "api.autoconnecto.in";
  otaCfg.deviceToken = config.deviceToken;
  otaCfg.rootCA = AUTOCONNECTO_ROOT_CA;
  otaCfg.softwarePath = "/ota/package.bin";
  otaCfg.autoReboot = false;

  ota.begin(otaCfg, [](const char* key, const char* value) {
    JsonDocument doc;
    doc[key] = value;
    return sdk.sendClientAttributes(doc);
  });
  ota.onProgressReport([](const char* stateKey, const char* state, const char* title, const char* version) {
    JsonDocument doc;
    doc[stateKey] = state;
    doc["sw_title"] = title;
    doc["sw_version"] = version;
    return sdk.sendClientAttributes(doc);
  });

  ota.onSoftwareReady(
    [](const String& path, const String& title, const String& version) {
      Serial.printf(
        "[SOTA] ready path=%s title=%s version=%s\n",
        path.c_str(),
        title.c_str(),
        version.c_str()
      );
      if (LittleFS.begin(true)) {
        File f = LittleFS.open(path, "r");
        if (f) {
          Serial.printf("[SOTA] file size on disk=%u\n", (unsigned)f.size());
          f.close();
        }
      }
    }
  );

  sdk.onAttributeStringUpdate([](const String& key, const String& value) {
    ota.onSharedAttribute(key, value);
  });
  sdk.onAttributeUpdate([](const String& key, float value) {
    ota.onSharedAttribute(key, value);
  });
  sdk.onConnect([](bool ok) {
    if (ok) {
      sdk.requestSharedAttributes(
        "sw_title,sw_version,sw_size,sw_checksum,sw_checksum_algorithm"
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
      "sw_title,sw_version,sw_size,sw_checksum,sw_checksum_algorithm"
    );
  }
}
