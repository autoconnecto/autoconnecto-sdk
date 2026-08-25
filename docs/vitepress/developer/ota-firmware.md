---
title: OTA firmware updates
description: Over-the-air firmware updates using ThingsBoard-style shared attributes and chunked HTTPS download.
---

# OTA firmware updates

Autoconnecto OTA follows the [ThingsBoard OTA model](https://thingsboard.io/docs/user-guide/ota-updates/):

1. Platform stores firmware metadata in **shared attributes** on the device.
2. Device detects new firmware title/version/size.
3. Device downloads binary in **chunks** over HTTPS.
4. Device flashes (ESP32: `esp_ota`) and reports status via client attributes.

## ESP32 example

Sketch: [`OtaFirmwareUpdate_mqtt`](https://github.com/autoconnecto/autoconnecto-sdk/tree/main/examples/OtaFirmwareUpdate_mqtt)

Uses `AutoconnectoSDK` for MQTT attribute notifications and `AutoconnectoOta` for chunked download.

## HTTPS download API

```http
GET https://api.autoconnecto.in/api/v1/{deviceToken}/firmware?title=&version=&size=&chunk=
```

Each chunk returns a slice of the firmware binary. Non-ESP32 devices implement their own flash/write logic using the same API.

## Shared attributes (platform → device)

Typical keys (ThingsBoard-compatible):

| Key | Description |
|-----|-------------|
| `fw_title` | Firmware title |
| `fw_version` | Target version |
| `fw_size` | Total bytes |
| `fw_checksum` | Optional integrity check |

## Production notes

- OTA binary storage uses platform S3 in production (see deployment docs).
- Keep MQTT connected during download for progress/status attributes, or poll via HTTPS.
- Test with a staging device profile before fleet rollout.

Full SDK contract: [OTA.md](https://github.com/autoconnecto/autoconnecto-sdk/blob/main/OTA.md)
