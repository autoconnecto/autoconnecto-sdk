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

Report state with a JSON client-attributes document (string values):

```cpp
ota.begin(cfg, [](const char* key, const char* value) {
  JsonDocument doc;
  doc[key] = value;
  return sdk.sendClientAttributes(doc);
});
```

Call `ota.loop()` from `loop()` while the SDK runs.

## Device download API (any platform)

```
GET https://{apiHost}/api/v1/{deviceToken}/firmware
  ?title={title}&version={version}&size={chunkBytes}&chunk={zeroBasedIndex}

GET https://{apiHost}/api/v1/{deviceToken}/software
  ?title={title}&version={version}&size={chunkBytes}&chunk={zeroBasedIndex}
```

Response body: raw binary slice. Repeat until all bytes received. The platform only serves chunks for an **active assignment** for that device.

## Client attribute progress

```
POST / attributes (or MQTT client attributes)
{ "fw_state": "DOWNLOADING" }   // FOTA
{ "sw_state": "DOWNLOADING" }   // SOTA
```

States: `DOWNLOADING`, `DOWNLOADED`, `VERIFIED`, `UPDATING`, `UPDATED`, `FAILED`.

## Local platform dev

Backend may store packages on disk under `backend/.ota-firmware/` when `OTA_S3_BUCKET` is unset (non-production). Production requires S3 — see `backend/ENVIRONMENT.md`.
