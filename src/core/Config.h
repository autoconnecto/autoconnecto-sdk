#pragma once

#include <Arduino.h>

#include "../AutoconnectoIsrgRoots.h"
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

  /** true: scan every channel and join the strongest AP. false: first match found. */
  bool wifiJoinStrongest = true;

  /** Give up the Wi-Fi join after this long. Default 15 s. */
  unsigned long wifiConnectTimeoutMs = 15000;

  /**
   * Test hook. 0 = off.
   * N = fail the next N MQTT connect attempts in software (no socket),
   * then connect. Each failure flips WSS/MQTTS and arms backoff.
   */
  uint8_t injectConnectFailures = 0;

  /**
   * After this many failed MQTT reconnect attempts (and when the link is
   * down), tear down and rejoin Wi-Fi so DNS/routing refresh. 0 = never.
   * Default 3 (~ tens of seconds with backoff).
   */
  uint8_t mqttWifiRefreshEveryFailures = 3;

  /**
   * If MQTT stays down this long, call esp_restart(). 0 = never.
   * Default 3 minutes. Shared-attribute restore still runs on the next connect.
   */
  unsigned long mqttRestartAfterDisconnectMs = 180000UL;

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

  uint16_t wssPort = 8084;

  // =========================================
  // DEVICE
  // =========================================

  String deviceToken;

  // =========================================
  // TRANSPORT
  // =========================================

  bool enableMQTT = true;

  bool enableWS = true;

  /** false = mqtt:// / ws:// (local EMQX :1883 / :8083). true = mqtts:// / wss:// */
  bool mqttUseTls = true;

  // Optional comma-separated SHARED keys for pull on connect (empty = all).
  String sharedAttributeKeys = "";

  // =========================================
  // TLS
  // =========================================

  bool allowInsecureTLS = false;

  const char* rootCA = AUTOCONNECTO_ROOT_CA;

  // =========================================
  // TELEMETRY
  // =========================================

  unsigned long telemetryIntervalMs = 5000;

  // =========================================
  // DEBUG
  // =========================================

  bool enableSerialLogs = true;
};