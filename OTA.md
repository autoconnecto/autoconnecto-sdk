# OTA (firmware and software)

Autoconnecto OTA uses shared-attribute notification + HTTPS package download:

1. Upload a package in the web UI (**OTA** menu) — **Firmware (FOTA)** or **Software (SOTA)**.
2. Assign to a device, profile, or device type.
3. Platform writes shared attributes:
   - FOTA: `fw_title`, `fw_version`, `fw_size`, `fw_checksum`, `fw_checksum_algorithm`
   - SOTA: `sw_title`, `sw_version`, `sw_size`, `sw_checksum`, `sw_checksum_algorithm`
4. Device downloads the package and reports client attribute `fw_state` or `sw_state`.

## ESP32 helpers (`AutoconnectoOta`)

Header: `OtaUpdate.h` (`#include <OtaUpdate.h>`).

| Kind | Shared attrs | Download API | Progress attr | Device action |
|------|--------------|--------------|---------------|---------------|
| **Firmware** | `fw_*` | `/api/v1/{token}/firmware` | `fw_state` | `esp_ota` flash + optional reboot |
| **Software** | `sw_*` | `/api/v1/{token}/software` | `sw_state` | Write file on **LittleFS** (default `/ota/package.bin`); optional `onSoftwareReady` callback; **no** reboot |

Both paths use SHA256 verification when `*_checksum_algorithm` is `SHA256`.

### Examples

- `examples/OtaFirmwareUpdate_mqtt/` — FOTA only
- `examples/OtaSoftwareUpdate_mqtt/` — SOTA only (prints path when ready)

Wire shared attributes into the helper:

```cpp
sdk.onAttributeStringUpdate([](const String& key, const String& value) {
  ota.onSharedAttribute(key, value);
});
sdk.onAttributeUpdate([](const String& key, float value) {
  ota.onSharedAttribute(key, value); // fw_size / sw_size as number
});
```

A new title or version clears the previous size and checksum. The download starts from `ota.loop()`, not from the attribute callback. After `UPDATED` or `FAILED` those fields are cleared. A version already installed on that device is not downloaded again.

Report state, title, and version in one client-attribute write:

```cpp
ota.onProgressReport([](const char* stateKey, const char* state, const char* title, const char* version) {
  JsonDocument doc;
  doc[stateKey] = state;
  doc["fw_title"] = title;
  doc["fw_version"] = version;
  return sdk.sendClientAttributes(doc);
});
```

Call `ota.loop()` from `loop()` while the SDK runs.

On connect, request the firmware keys by name. An empty shared-attribute pull can be skipped for 15 seconds after a snapshot. A named request is not skipped.

```cpp
sdk.requestSharedAttributes("fw_title,fw_version,fw_size,fw_checksum,fw_checksum_algorithm");
```

Chunks reuse one TLS connection. Each response is still one chunk.

After an OTA install the running slot is `app0` or `app1` and the boot state is `PENDING_VERIFY` until the sketch confirms it. `examples/OtaHealthCheck_mqtt/` defines `verifyRollbackLater()` so the image stays `PENDING_VERIFY` until `markValidAfterHealthyCheck()`. Without that function the chip marks the image valid as soon as the download ends. After a USB flash the state is `UNDEFINED` and `markValidAfterHealthyCheck()` does nothing. `rollbackIfUnhealthy()` reboots to the previous image only while the state is `PENDING_VERIFY`. A stale `sw_*` set in the same push as a complete `fw_*` set does not cancel the firmware download. `fail()` also publishes `fw_error` or `sw_error` with the serial reason.

A boot report of `fw_version` or `sw_version` without `fw_state` is the image the chip is running. If that differs from a target the platform already marked updated, the platform fails that update and clears the shared target.

## Device download API (any platform)

```
GET https://{apiHost}/api/v1/{deviceToken}/firmware
  ?title={title}&version={version}&size={chunkBytes}&chunk={zeroBasedIndex}

GET https://{apiHost}/api/v1/{deviceToken}/software
  ?title={title}&version={version}&size={chunkBytes}&chunk={zeroBasedIndex}
```

Response body: raw binary slice for **that chunk only** (`Content-Length` is the slice size, not the whole package). The device must finish reading the slice, then request `chunk+1`, until bytes written ≥ `fw_size` / `sw_size`. Do not wait on one HTTP connection for the entire file — that hangs. The platform only serves chunks for an **active assignment** for that device.

## Client attribute progress

```
POST / attributes (or MQTT client attributes)
{ "fw_state": "DOWNLOADING", "fw_title": "pump", "fw_version": "1.2.0" }
{ "sw_state": "DOWNLOADING", "sw_title": "pump", "sw_version": "1.2.0" }
```

States: `DOWNLOADING`, `DOWNLOADED`, `VERIFIED`, `UPDATING`, `UPDATED`, `FAILED`.

## Local platform dev

The server stores packages on its own disk at `/var/lib/autoconnecto/ota`. That folder is kept when the software is updated. S3 is optional. See `backend/ENVIRONMENT.md`.
