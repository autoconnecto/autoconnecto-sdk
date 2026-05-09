#pragma once

#include "Types.h"

struct RuntimeState {

  // =====================================
  // WIFI
  // =====================================

  bool wifiConnected = false;

  // =====================================
  // MQTT
  // =====================================

  bool mqttConnected = false;

  // =====================================
  // ACTIVE TRANSPORT
  // =====================================

  TransportType activeTransport =
    TRANSPORT_NONE;

  // =====================================
  // TIMERS
  // =====================================

  unsigned long lastTelemetryAt = 0;

  unsigned long lastReconnectAt = 0;

  // =====================================
  // CONNECTION STATE
  // =====================================

  ConnectionState state =
    STATE_DISCONNECTED;
};