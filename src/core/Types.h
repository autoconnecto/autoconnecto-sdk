#pragma once

#include <Arduino.h>

enum TransportType {
  TRANSPORT_NONE,
  TRANSPORT_MQTT,
  TRANSPORT_WS
};

enum ConnectionState {
  STATE_DISCONNECTED,
  STATE_CONNECTING,
  STATE_CONNECTED,
  STATE_AUTHENTICATED
};