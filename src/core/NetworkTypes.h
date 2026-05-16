#pragma once

#include <Arduino.h>

// Network link used before MQTT. Default WiFi — existing sketches unchanged.
enum class NetworkMode : uint8_t {
  WiFi = 0,
  LtePpp = 1,
};
