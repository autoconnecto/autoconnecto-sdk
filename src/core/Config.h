#pragma once

#include <Arduino.h>

struct SDKConfig {

  // WiFi
  String wifiSSID;
  String wifiPassword;

  // MQTT
  String mqttHost;
  uint16_t mqttPort = 1883;

  // WS
  String wsHost;
  uint16_t wsPort = 3001;
  String wsPath = "/";

  // Device
  String deviceToken;

  // Runtime
  bool enableMQTT = true;
  bool enableWS = true;

  bool preferWS = true;
  bool mqttFallback = true;

  // Telemetry
  unsigned long telemetryIntervalMs = 5000;

  // Debug
  bool enableSerialLogs = true;
};