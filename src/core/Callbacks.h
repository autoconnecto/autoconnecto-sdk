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