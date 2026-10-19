# Changelog

This repository ships the **Autoconnecto Arduino SDK** (`src/`, `examples/`) and a **Raspberry Pi examples** sibling (`raspberrypi/`).

Library Manager users receive tagged releases (`vX.Y.Z`) of the Arduino library. Raspberry Pi examples track the same git history.

## v1.7.54

- OTA clears title, version, size, and checksum after UPDATED or FAILED. A changed title or version drops the previous size and checksum. The download runs from `loop()`. A version already stored on the device is not downloaded again. Progress reports send title and version with the state.
- `SDKConfig` defaults: `wssPort` 8084, `allowInsecureTLS` false, `rootCA` `AUTOCONNECTO_ROOT_CA`.
- Shared-attribute JSON parses an 8 KB document. A text value that is not a number is not also delivered as 0.
- `TelemetryBatch_mqtt` waits 1100 ms between samples and prints a failed publish.
- LTE `build_opt.h` is one `-D` line. Core 3.3 uses `PPP_MODEM_GENERIC` for the EC200 AT set.
- HTTPS examples for ESP8266 and Raspberry Pi Pico W. The ESP32 MQTT client is not compiled on those boards.

## Unreleased

- Add OTA client (`src/ota/OtaUpdate.h`) and example `examples/OtaFirmwareUpdate_mqtt/`.
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

