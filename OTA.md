# OTA (over-the-air firmware)

Autoconnecto OTA follows the [ThingsBoard OTA model](https://thingsboard.io/docs/user-guide/ota-updates/):

1. Upload a package in the web UI (**OTA** menu).
2. Assign to a device, profile, or device type.
3. Platform writes shared attributes: `fw_title`, `fw_version`, `fw_size`, `fw_checksum`, `fw_checksum_algorithm`.
4. Device downloads firmware and reports client attribute `fw_state`.

## ESP32 Arduino example

`examples/OtaFirmwareUpdate_mqtt/OtaFirmwareUpdate_mqtt.ino`

Uses `AutoconnectoSDK` for MQTT + `AutoconnectoOta` for HTTPS chunked download and `esp_ota` flash.

## Device download API (any platform)

```
GET https://{apiHost}/api/v1/{deviceToken}/firmware
  ?title={title}
  &version={version}
  &size={chunkBytes}
  &chunk={zeroBasedIndex}
```

Response body: raw binary slice. Repeat until all bytes received.

Software (SOTA) packages use `/software` instead of `/firmware`.

## Client attribute progress

Post JSON to:

```
POST https://{apiHost}/api/v1/{deviceToken}/attributes
{ "fw_state": "DOWNLOADING" }
```

States: `DOWNLOADING`, `DOWNLOADED`, `VERIFIED`, `UPDATING`, `UPDATED`, `FAILED`.

## Local platform dev

Backend may store packages on disk under `backend/.ota-firmware/` when `OTA_S3_BUCKET` is unset (non-production). Production requires S3 — see `backend/ENVIRONMENT.md`.
