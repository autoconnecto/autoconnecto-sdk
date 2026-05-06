#include "Logger.h"

bool Logger::_enabled = true;

void Logger::begin(bool enabled) {
  _enabled = enabled;
}

void Logger::info(const String& msg) {
  if (!_enabled) return;

  Serial.print("[INFO] ");
  Serial.println(msg);
}

void Logger::warn(const String& msg) {
  if (!_enabled) return;

  Serial.print("[WARN] ");
  Serial.println(msg);
}

void Logger::error(const String& msg) {
  if (!_enabled) return;

  Serial.print("[ERROR] ");
  Serial.println(msg);
}