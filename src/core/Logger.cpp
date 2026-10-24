#include "Logger.h"

bool Logger::_enabled = true;

void Logger::begin(bool enabled) {
  _enabled = enabled;
}

String Logger::redactSecrets(const String& msg) {
  // MQTT topics are devices/{deviceToken}/... — never print the token.
  String out = msg;
  int from = 0;
  while (true) {
    int idx = out.indexOf("devices/", from);
    if (idx < 0) break;
    int tokenStart = idx + 8; // after "devices/"
    int tokenEnd = out.indexOf('/', tokenStart);
    if (tokenEnd < 0) {
      // Topic may be the whole remainder (rare).
      out = out.substring(0, tokenStart) + "***";
      break;
    }
    out =
      out.substring(0, tokenStart) +
      "***" +
      out.substring(tokenEnd);
    from = tokenStart + 3 + 1; // past "***" and '/'
  }
  return out;
}

void Logger::info(const String& msg) {
  if (!_enabled) return;

  Serial.print("[INFO] ");
  Serial.println(redactSecrets(msg));
}

void Logger::warn(const String& msg) {
  if (!_enabled) return;

  Serial.print("[WARN] ");
  Serial.println(redactSecrets(msg));
}

void Logger::error(const String& msg) {
  if (!_enabled) return;

  Serial.print("[ERROR] ");
  Serial.println(redactSecrets(msg));
}

void Logger::note(const String& msg) {
  if (!_enabled) return;

  Serial.println(redactSecrets(msg));
}
