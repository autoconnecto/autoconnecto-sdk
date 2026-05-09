#pragma once

#include <Arduino.h>

// =========================================
// TRANSPORT TYPE
// =========================================

enum TransportType {

  TRANSPORT_NONE,

  TRANSPORT_MQTT
};

// =========================================
// CONNECTION STATE
// =========================================

enum ConnectionState {

  STATE_DISCONNECTED,

  STATE_CONNECTING,

  STATE_CONNECTED
};