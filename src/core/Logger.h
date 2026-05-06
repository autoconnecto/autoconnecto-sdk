#pragma once

#include <Arduino.h>

class Logger {
public:

  static void begin(bool enabled);

  static void info(const String& msg);

  static void warn(const String& msg);

  static void error(const String& msg);

private:

  static bool _enabled;
};