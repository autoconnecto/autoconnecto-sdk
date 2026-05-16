#pragma once

#include <Arduino.h>

#include "NetworkTypes.h"

struct SDKConfig {

  // =========================================
  // NETWORK (default: WiFi — unchanged for existing code)
  // =========================================

  NetworkMode networkMode = NetworkMode::WiFi;

  // =========================================
  // WIFI
  // =========================================

  String wifiSSID;

  String wifiPassword;

  // =========================================
  // LTE PPP (EC200) — used when networkMode == LtePpp
  // UART defaults: ESP32 Serial2 RX=16, TX=17
  // =========================================

  String lteApn = "internet";

  String ltePin = "";

  int lteUartTx = 17;

  int lteUartRx = 16;

  int lteRtsPin = -1;

  int lteCtsPin = -1;

  int lteResetPin = -1;

  bool lteResetActiveLow = true;

  int lteResetDelayMs = 200;

  unsigned long lteAttachTimeoutMs = 120000;

  // =========================================
  // MQTT / WSS HOST
  // =========================================

  String mqttHost;

  // =========================================
  // PORTS
  // =========================================

  uint16_t mqttPort = 8883;

  uint16_t wssPort = 443;

  // =========================================
  // DEVICE
  // =========================================

  String deviceToken;

  // =========================================
  // TRANSPORT
  // =========================================

  bool enableMQTT = true;

  bool enableWS = true;

  // =========================================
  // TLS
  // =========================================

  bool allowInsecureTLS = true;

  const char* rootCA = nullptr;

  // =========================================
  // TELEMETRY
  // =========================================

  unsigned long telemetryIntervalMs = 5000;

  // =========================================
  // DEBUG
  // =========================================

  bool enableSerialLogs = true;
};