#pragma once

// LTE PPP (Quectel EC200, etc.) is opt-in at compile time.
// Default 0: library build matches pre-LTE behaviour (WiFi only).
// Enable in sketch folder build_opt.h: -DAUTOCONNECTO_ENABLE_LTE_PPP=1
// Requires ESP32 Arduino core 3.x (built-in PPP library).
#ifndef AUTOCONNECTO_ENABLE_LTE_PPP
#define AUTOCONNECTO_ENABLE_LTE_PPP 0
#endif
