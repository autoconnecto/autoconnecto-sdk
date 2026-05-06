#pragma once

#include <Arduino.h>

class ITransport {
public:

  virtual bool begin() = 0;

  virtual void loop() = 0;

  virtual bool connected() = 0;

  virtual bool sendTelemetry(const String& payload) = 0;

  virtual bool sendClientAttribute(
    const String& key,
    float value
  ) = 0;

  virtual void requestAttributes() = 0;
};