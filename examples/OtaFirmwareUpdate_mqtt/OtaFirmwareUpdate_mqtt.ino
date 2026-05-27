// =============================================================
// OtaFirmwareUpdate_mqtt — ThingsBoard-style OTA (FOTA)
//
// PURPOSE
//   Connect via AutoconnectoSDK (MQTT/WSS). When the platform assigns
//   firmware, shared attributes fw_* arrive via onAttributeUpdate.
//   This sketch downloads the .bin in chunks over HTTPS and flashes
//   the ESP32, reporting fw_state as a client attribute.
//
// PLATFORM FLOW (dashboard OTA page)
//   Assign package → INITIATED (fw_* on device) → device reports
//   DOWNLOADING … UPDATED
//
// REQUIREMENTS
//   - Assign firmware with matching title/version (e.g. main / 1.0.1)
//   - API_HOST must be reachable from the ESP32 (LAN IP or public host;
//     not localhost from the device)
//   - ESP32 partition scheme with OTA slot (e.g. "Minimal SPIFFS" + OTA)
//
// NOTE
//   Raspberry Pi / HTTP-only devices: see sdk/OTA.md for the REST chunk API.
// =============================================================

#include <AutoconnectoSDK.h>
#include <OtaUpdate.h>

AutoconnectoSDK sdk;
AutoconnectoOta ota;

// ---- Edit for your network, API, and device -----------------------------

static const char* WIFI_SSID     = "YOUR_WIFI_SSID";
static const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
static const char* MQTT_HOST     = "mqtt.autoconnecto.in";
static const char* API_HOST      = "api.autoconnecto.in";
static const char* DEVICE_TOKEN  = "YOUR_DEVICE_TOKEN";

// Local dev: use your PC LAN IP if API is proxied locally, e.g. "192.168.1.10"
// and ensure HTTPS port matches your backend / reverse proxy.

static bool reportFwState(const char* key, const char* value) {
  StaticJsonDocument<128> doc;
  doc[key] = value;
  return sdk.sendClientAttributes(doc);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  OtaConfig otaCfg;
  otaCfg.apiHost = API_HOST;
  otaCfg.deviceToken = DEVICE_TOKEN;
  otaCfg.rootCA = AUTOCONNECTO_ROOT_CA;
  otaCfg.allowInsecureTLS = false;
  otaCfg.chunkSize = 16384;
  otaCfg.autoReboot = true;

  ota.begin(otaCfg, reportFwState);

  sdk.onAttributeUpdate([](const String& key, JsonVariant value) {
    ota.onSharedAttribute(key, value);
  });

  SDKConfig config;
  config.wifiSSID = WIFI_SSID;
  config.wifiPassword = WIFI_PASSWORD;
  config.mqttHost = MQTT_HOST;
  config.mqttPort = 8883;
  config.wssPort = 8084;
  config.deviceToken = DEVICE_TOKEN;
  config.enableMQTT = true;
  config.enableWS = true;
  config.allowInsecureTLS = false;
  config.rootCA = AUTOCONNECTO_ROOT_CA;
  config.enableSerialLogs = true;

  sdk.begin(config);

  // Pull any fw_* already assigned before this boot
  sdk.requestSharedAttributes(
    "fw_title,fw_version,fw_size,fw_checksum,fw_checksum_algorithm"
  );

  Serial.println("[OTA] OtaFirmwareUpdate_mqtt ready");
}

void loop() {
  sdk.loop();
  ota.loop();

  static unsigned long lastTelemetry = 0;
  if (millis() - lastTelemetry > 30000 && sdk.connected() && !ota.isBusy()) {
    lastTelemetry = millis();
    sdk.sendTelemetry("uptime_sec", millis() / 1000.0f);
  }
}
