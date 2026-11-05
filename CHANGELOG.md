# Changelog

This repository ships the **Autoconnecto Arduino SDK** (`src/`, `examples/`), an installable **Python package** (`python/` — `pip install -e ./python`, name **`autoconnecto`**), and **Linux SBC examples** (`raspberrypi/`).

Library Manager users receive tagged releases (`vX.Y.Z`) of the Arduino library. Python and Raspberry Pi examples track the same git history.

## v1.7.100

- Integration webhook examples: document gateway child `parentDeviceId`; Modbus/OPC-UA default poll 30 s; Modbus uses pymodbus `count=`/`device_id=` with `slave=` fallback, logs read errors, skips ts-only publish.
- `python/pyproject.toml` re-saved as UTF-8 (cp1252 em dash broke `pip install`); CI `utf8-check` rejects non-UTF-8 text files.
- Raspberry Pi examples: telemetry every 30 s (Free plan); `sdkVersion` string from one constant; shared attributes requested once on MQTT connect.
- `DeepSleep_http/secrets.h.example`: default `SLEEP_SEC` 300.
- Code review + static only — not board-verified.

## v1.7.85

- MQTT recovery after a Wi‑Fi/router flap: after repeated connect failures (default every 3), or when the link is down, the SDK rejoins Wi‑Fi (fresh DNS) or re-attaches LTE PPP, then rebuilds the MQTT/TLS client. If MQTT stays down for 3 minutes (configurable), it calls `esp_restart()`. Shared-attribute restore on reconnect is unchanged. Config: `mqttWifiRefreshEveryFailures`, `mqttRestartAfterDisconnectMs`. BOARD_VERIFY **B8** (pull router power ~60 s). Code review + static only — not board-verified.

## v1.7.79

- Installable Python SDK under `python/` (package **`autoconnecto`**): MQTT + HTTPS + GPIO (`gpio.write` / `read` / `set_relay`, extra `gpiozero`). Named SBCs: Raspberry Pi 5/4/3/Zero 2 W, Orange Pi 5, Radxa ROCK 5, BeagleBone. `raspberrypi/common` is a shim; examples import `autoconnecto`. Install: `pip install -e "./python[gpio,dotenv]"` (PyPI publish still pending). Code review + static only — not board-verified on a Pi.
- Arduino library version aligned to 1.7.79 (no ESP32 behaviour change in this tag).

## v1.7.78

- A complete `fw_*` set is not cleared when stale `sw_*` keys arrive in the same push. `fail()` publishes `fw_error` or `sw_error` with the serial reason only (not a bare `fw_version`, which the platform would read as a boot report). `fw_error` / `sw_error` are cleared after UPDATED and when a new download starts. `examples/OtaHealthCheck_mqtt` defines `verifyRollbackLater()` so a copy stays PENDING_VERIFY until the first telemetry. Code review + static only — not board-verified.

## v1.7.77

- A named shared-attribute request is sent even when a snapshot arrived in the last 15 seconds. An empty pull is still skipped.
- Firmware and software chunks reuse one TLS connection. Each response is still one chunk.
- `markValidAfterHealthyCheck`, `rollbackIfUnhealthy`, `bootPartitionLabel`, and `bootStateName`. After a USB flash the state is UNDEFINED and mark-valid does nothing. Example `OtaHealthCheck_mqtt`. Code review + static only — not board-verified.

## v1.7.76

- Shared attributes: one retained push on boot. A pull is not sent when that push already arrived. The broker client does not auto-reconnect on top of the SDK backoff. An unchanged snapshot is not published again (server v1.7.76).
- `onWifiJoin` / `lastWifiJoin` report BSSID, channel, and RSSI when serial logs are off.
- `injectConnectFailures` and `examples/MqttConnectFault_mqtt` fail connects in software so the transport switch and backoff can be checked.
- `onMqttRefuse` fires on MQTT CONNACK 4 or 5 (token refused). Code review + static only — not board-verified.

## v1.7.74

- Package text is plain UTF-8. `library.properties` sentence no longer contains `???`. Architectures list ESP32, the board that has been tested. `library.json` is the PlatformIO manifest (`lib_deps = autoconnecto/AutoconnectoSDK`).
- Wi-Fi join defaults to a full-channel scan sorted by signal (`wifiJoinStrongest`). Connect attempt times out (default 15 s). One `[WIFI] joined` line names BSSID, channel, and RSSI. A failed MQTT connect switches WSS/MQTTS and backs off. Identical shared attributes within 15 s are applied once. `enableSerialLogs=false` is honoured. Examples: `WifiJoinStrongest_mqtt`, `Attributes_RestoreOnBoot_mqtt`. Code review + static only — not board-verified.
- `Claim_https` accepts HTTP 201 (any 2xx) and stores the token. A failure prints the server message only. The token and the claim code are not printed. `esp32-claim` no longer prints the handoff body. Code review + static only — not board-verified.

## v1.7.73

- Logger redacts device tokens in MQTT topic INFO/WARN/ERROR lines (`devices/***/�`).
- OfflineBuffer_mqtt: NTP wall-clock timestamps; skip queue until time is synced (no 1970 dates).


## v1.7.68

- Example `examples/esp32-claim`: the sketch sends a claim code and receives a device token once. The QR holds the code, not the token. Not board-verified.

## v1.7.58

- **OTA:** firmware/software download no longer waits forever for the whole file in one HTTP request. Each response is one chunk (`Content-Length` / idle timeout); then the next `chunk=` index is fetched until size is met.
- **Examples:** Switch widgets — handle shared `channelN` attrs and RPC `set_channel_state` (same as `relay_set`). `RPCCommands_mqtt` registers `onAttributeUpdate`.
- **Examples:** `telemetry_burst` arms a counter and sends on the normal 35 s interval (no 35 s `delay` inside RPC).
- **Examples:** remove hard-coded Wi-Fi credentials from `FleetSimulator_10_machines_http`.
- **Examples:** `WidgetCoverage_mqtt` for Phase 7 Solution keys.
- **Docs:** `BOARD_VERIFY.md` — board path must be proven before claiming fixed.
- Pair with platform: Command Log for `POST /api/devices/:id/commands`; MQTT shared-attribute publish even when the profile transport list omits MQTT.

## v1.7.57

- Release tag alignment with platform **v1.7.57** (security headers / audit ship). SDK code same as 1.7.56 content for examples/OTA callback order and Free-plan telemetry interval.

## v1.7.56

- README / `library.properties` version string aligned.
- MQTT examples: **one** telemetry JSON per interval (default **35 s**, Free-plan safe). Multiple `sendTelemetry` calls in the same second are dropped by plan `min_telemetry_interval_sec`.
- OTA examples register attribute/connect callbacks **before** `sdk.begin()`.
- `onAttributeUpdate` / `onAttributeStringUpdate` / `onRPC` re-apply into the transport if registered after `begin()`.
- OTA client (`src/ota/OtaUpdate.*`) + `OtaFirmwareUpdate_mqtt` / `OtaSoftwareUpdate_mqtt` + `OTA.md` (chunked download API).

## v1.7.54

- OTA clears title, version, size, and checksum after UPDATED or FAILED. A changed title or version drops the previous size and checksum. The download runs from `loop()`. A version already stored on the device is not downloaded again. Progress reports send title and version with the state.
- `SDKConfig` defaults: `wssPort` 8084, `allowInsecureTLS` false, `rootCA` `AUTOCONNECTO_ROOT_CA`.
- Shared-attribute JSON parses an 8 KB document. A text value that is not a number is not also delivered as 0.
- `TelemetryBatch_mqtt` waits 1100 ms between samples and prints a failed publish.
- LTE `build_opt.h` is one `-D` line. Core 3.3 uses `PPP_MODEM_GENERIC` for the EC200 AT set.
- HTTPS examples for ESP8266 and Raspberry Pi Pico W. The ESP32 MQTT client is not compiled on those boards.

## v1.0.5

- Arduino Library Manager compliance and release hardening.

## v1.0.3

- Release and documentation updates.

## v1.0.0

- Initial public release for Arduino Library Manager.

