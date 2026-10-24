#pragma once

#include <Arduino.h>

class Logger {
public:

  static void begin(bool enabled);

  static void info(const String& msg);

  static void warn(const String& msg);

  static void error(const String& msg);

  /** Print one line when logs are on. No [INFO] prefix. */
  static void note(const String& msg);

  /** Strip device tokens from MQTT topic strings before Serial print. */
  static String redactSecrets(const String& msg);

private:

  static bool _enabled;
};