#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

// =========================================
// ATTRIBUTE CALLBACK
// =========================================

typedef void (*AttributeCallback)(
  const String& key,
  float value
);

/** String SHARED attrs (e.g. machine_code) — numeric callback alone cannot carry these. */
typedef void (*AttributeStringCallback)(
  const String& key,
  const String& value
);

// =========================================
// CONNECTION CALLBACK
// =========================================

typedef void (*ConnectionCallback)(
  bool connected
);

/** Fires when the radio joins an access point. Not gated by enableSerialLogs. */
typedef void (*WifiJoinCallback)(
  const char* bssid,
  int channel,
  int rssi
);

struct WifiJoinInfo {
  bool valid;
  char bssid[18];
  int channel;
  int rssi;
};

/** MQTT CONNACK 4 or 5: the broker refused the device token. */
typedef void (*MqttRefuseCallback)(
  int returnCode
);

// =========================================
// LOG CALLBACK
// =========================================

typedef void (*LogCallback)(
  const String& message
);

// =========================================
// RPC CALLBACK
// =========================================

typedef void (*RPCCallback)(
  const String& method,
  JsonObject payload
);