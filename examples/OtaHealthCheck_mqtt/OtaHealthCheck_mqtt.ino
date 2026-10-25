// =============================================================
// OtaHealthCheck_mqtt
//
// After an OTA install the running slot is app0 or app1 and the
// boot state is PENDING_VERIFY until the sketch confirms it.
// After a USB flash the boot state is UNDEFINED. mark-valid
// does nothing in that case. Chip rollback exists only after
// an OTA install.
//
// On the first telemetry this sketch calls
// ota.markValidAfterHealthyCheck(). To exercise a bad image,
// call ota.rollbackIfUnhealthy() instead of mark-valid while
// the state is still PENDING_VERIFY.
// =============================================================

#include <AutoconnectoSDK.h>
#include <OtaUpdate.h>
#include <ArduinoJson.h>
#include "secrets.h"

// Leave the new image PENDING_VERIFY until markValidAfterHealthyCheck().
// Without this, the chip marks the image valid as soon as the download ends.
extern "C" bool verifyRollbackLater() { return true; }

AutoconnectoSDK sdk;
AutoconnectoOta ota;
bool confirmed = false;

void setup() {
  Serial.begin(115200);
  delay(1500);

  SDKConfig config;
  config.wifiSSID = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;
  config.deviceToken = DEVICE_TOKEN;
  config.mqttHost = "mqtt.autoconnecto.in";
  config.mqttPort = 8883;
  config.mqttUseTls = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
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

  Serial.printf(
    "[OTA] boot slot=%s state=%s\n",
    ota.bootPartitionLabel(),
    ota.bootStateName()
  );
  sdk.begin(config);
}

void loop() {
  sdk.loop();
  ota.loop();
  if (!confirmed && sdk.connected()) {
    JsonDocument doc;
    doc["health"] = 1;
    if (sdk.sendTelemetry(doc)) {
      confirmed = true;
      ota.markValidAfterHealthyCheck();
    }
  }
}
