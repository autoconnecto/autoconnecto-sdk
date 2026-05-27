# Changelog

This repository ships the **Autoconnecto Arduino SDK** (`src/`, `examples/`) and a **Raspberry Pi examples** sibling (`raspberrypi/`).

Library Manager users receive tagged releases (`vX.Y.Z`) of the Arduino library. Raspberry Pi examples track the same git history.

## Unreleased

- Add ThingsBoard-style OTA client (`src/ota/OtaUpdate.h`) and example `examples/OtaFirmwareUpdate_mqtt/`.
- Add `OTA.md` (chunked firmware download API).
- Add Raspberry Pi example suite under `raspberrypi/` mirroring the ESP32 example names.
- Add systemd unit template for Raspberry Pi examples.
- Add Raspberry Pi generator demo: `raspberrypi/examples/Generator_Monitoring_mqtt`.
- Add LTE PPP (EC200) minimal telemetry example: `BasicTelemetry_lte_ppp_mqtt`.
- Add buffered telemetry flush example over MQTT: `TelemetryBatch_mqtt`.
- Add watchdog-style reconnect example: `WatchdogReconnect_mqtt`.
- Add transport contract docs: `contracts/`.
- Add STM32 (Arduino core) guide and CubeIDE scaffold.
- Add gateway integration examples: OPC‑UA, Modbus, MQTT bridge.

## v1.0.5

- Arduino Library Manager compliance and release hardening.

## v1.0.3

- Release and documentation updates.

## v1.0.0

- Initial public release for Arduino Library Manager.

