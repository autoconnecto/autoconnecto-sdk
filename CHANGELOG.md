# Changelog

This repository ships the **Autoconnecto Arduino SDK** (`src/`, `examples/`) and a **Raspberry Pi examples** sibling (`raspberrypi/`).

Library Manager users receive tagged releases (`vX.Y.Z`) of the Arduino library. Raspberry Pi examples track the same git history.

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

- Logger redacts device tokens in MQTT topic INFO/WARN/ERROR lines (`devices/***/…`).
- OfflineBuffer_mqtt: NTP wall-clock timestamps; skip queue until time is synced (no 1970 dates).


## v1.7.68

- Example `examples/esp32-claim`: the sketch sends a claim code and receives a device token once. The QR holds the code, not the token. Not board-verified.

## v1.7.58

- **OTA:** firmware/software download no longer waits forever for the whole file in one HTTP request. Each response is one chunk (`Content-Length` / idle timeout); then the next `chunk=` index is fetched until size is met.
- **Examples:** Switch widgets â€” handle shared `channelN` attrs and RPC `set_channel_state` (same as `relay_set`). `RPCCommands_mqtt` registers `onAttributeUpdate`.
- **Examples:** `telemetry_burst` arms a counter and sends on the normal 35 s interval (no 35 s `delay` inside RPC).
- **Examples:** remove hard-coded Wi-Fi credentials from `FleetSimulator_10_machines_http`.
- **Examples:** `WidgetCoverage_mqtt` for Phase 7 Solution keys.
- **Docs:** `BOARD_VERIFY.md` â€” board path must be proven before claiming fixed.
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

