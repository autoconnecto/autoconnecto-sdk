#pragma once

#include "Types.h"

struct RuntimeState {

  bool wifiConnected = false;

  bool mqttConnected = false;
  bool wsConnected = false;
  bool wsAuthed = false;

  TransportType activeTransport = TRANSPORT_NONE;

  unsigned long lastTelemetryAt = 0;
  unsigned long lastReconnectAt = 0;

  ConnectionState state = STATE_DISCONNECTED;
};