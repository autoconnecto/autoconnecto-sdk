#pragma once

// MQTT, WiFi, and the ESP32 OTA client are not compiled for ESP8266 or RP2040.
// Those boards use the HTTPS examples and AutoconnectoIsrgRoots.h.
#if !defined(ESP8266) && !defined(ARDUINO_ARCH_RP2040)
#define AUTOCONNECTO_ESP32_CLIENT 1
#endif
