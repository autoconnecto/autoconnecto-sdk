#pragma once

#include <Arduino.h>

typedef void (*AttributeCallback)(
  const String& key,
  float value
);

typedef void (*ConnectionCallback)(
  bool connected
);

typedef void (*LogCallback)(
  const String& message
);