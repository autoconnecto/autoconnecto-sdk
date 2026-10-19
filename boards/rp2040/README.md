# Raspberry Pi Pico W

HTTP first. The ESP32 MQTT client is not compiled for this board.

Sketch: `examples/BasicTelemetry_pico_w_http`

- Core: Raspberry Pi Pico/RP2040 (Earle Philhower)
- Board: Raspberry Pi Pico W
- TLS uses `AutoconnectoIsrgRoots.h`
- Telemetry is `POST /api/v1/{deviceToken}/telemetry`
- A plan refusal is HTTP 429. The sketch prints the body.

MQTT-over-WSS stays on ESP32. A Pico W on the bench is what proves this sketch. Compiling it is not that proof.
