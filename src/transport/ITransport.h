#pragma once

#include <Arduino.h>

class ITransport {

public:

  virtual bool begin() = 0;

  virtual void loop() = 0;

  virtual bool connected() = 0;

  // =====================================================
  // TELEMETRY
  // =====================================================

  virtual bool sendTelemetry(
    const String& payload
  ) = 0;

  // =====================================================
  // CLIENT ATTRIBUTES
  // =====================================================

  virtual bool sendClientAttribute(
    const String& key,
    float value
  ) = 0;

  virtual bool sendClientAttributes(
    const String& payload
  ) = 0;

  // =====================================================
  // SHARED ATTRIBUTES
  // =====================================================

  virtual void requestAttributes() = 0;

  virtual void requestAttributes(
    const String& keys
  ) = 0;

  // =====================================================
  // RPC
  // =====================================================

  virtual bool sendRPCResponse(
    const String& payload
  ) = 0;
};