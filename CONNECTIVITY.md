# Device connectivity (Autoconnecto SDK & integrations)

This guide covers **confirmed** ingress paths for Autoconnecto v1.2.6+. Arduino sketches live under `examples/`; server-side integration samples live under `examples/integrations/`.

## Choose a path

| Your device / system | Example | Auth |
|----------------------|---------|------|
| ESP32 firmware (recommended) | `BasicTelemetry_mqtt` | Device token (MQTT username) |
| ESP32 diesel generator demo | `Generator_Monitoring_mqtt` | Device token; publishes `gen_*` keys for **Generator Monitoring** widget |
| ESP32 over HTTPS only | `BasicTelemetry_http` | Device token in URL |
| Edge gateway relaying children | `GatewayRelay_http` | Gateway device token |
| Buffered / bulk HTTP upload | `TelemetryBatch_http` | Device token in URL |
| External cloud (IFTTT, ERP, script) | `integrations/generic-webhook.*` | Tenant webhook secret |
| ChirpStack LoRaWAN | `integrations/chirpstack-*` | Webhook secret + DevEUI on device |
| TTN / The Things Stack v3 | `integrations/ttn-*` | Webhook secret + DevEUI on device |

Platform UI: **Device Details → Check connectivity** (per-device curl/MQTT/WS) and **Tenant Settings → LoRa & integration webhooks** (tenant URLs and secret).

## Direct device (HTTP)

```http
POST https://api.autoconnecto.in/api/v1/{deviceToken}/telemetry
Content-Type: application/json

{"temperature": 24.5, "humidity": 61}
```

Arduino: `examples/BasicTelemetry_http/BasicTelemetry_http.ino`

Optional CBOR: `Content-Type: application/cbor` with a CBOR-encoded object (same keys as JSON).

## Direct device (MQTT)

Publish JSON to:

```text
devices/{deviceToken}/telemetry
```

ThingsBoard-compatible topic (token in path):

```text
v1/devices/{deviceToken}/telemetry
```

Arduino: `examples/BasicTelemetry_mqtt/BasicTelemetry_mqtt.ino`

## HTTP telemetry batch

Up to **100** samples per request:

```http
POST https://api.autoconnecto.in/api/v1/{deviceToken}/telemetry/batch
Content-Type: application/json

{
  "items": [
    { "ts": 1712486400000, "telemetry": { "temperature": 21 } },
    { "telemetry": { "temperature": 22 } }
  ]
}
```

Arduino: `examples/TelemetryBatch_http/TelemetryBatch_http.ino`

## Gateway relay

1. Create a **gateway** device (Mark as Gateway Device).
2. Create **child** devices with parent set to that gateway.
3. Publish using the **gateway token** and include the child id:

```json
{
  "childDeviceId": "87533b15-c04a-4c27-bb2a-adcdaf2444f4",
  "telemetry": { "temperature": 24.5 }
}
```

HTTP uses the same JSON body on `POST /api/v1/{gatewayToken}/telemetry`.

Arduino: `examples/GatewayRelay_http/GatewayRelay_http.ino`

## Generic integration webhook

For systems that POST JSON to your tenant (not ChirpStack/TTN native format):

```http
POST https://api.autoconnecto.in/api/v1/integrations/generic/telemetry?tenantId={tenant-uuid}
X-Webhook-Secret: {secret}
Content-Type: application/json
```

By **device token**:

```json
{
  "deviceToken": "your-device-access-token",
  "temperature": 22.5,
  "humidity": 60
}
```

By **device id** (auto-registered as `integration.external_id` on create):

```json
{
  "externalDeviceId": "87533b15-c04a-4c27-bb2a-adcdaf2444f4",
  "telemetry": { "temperature": 22.5 }
}
```

Flat telemetry (no nested `telemetry` object) is also accepted — reserved keys are `deviceToken`, `externalDeviceId`, `tenantId`, `childDeviceId`, `ts`, `timestamp`.

Samples: `examples/integrations/README.md`, `generic-webhook.sh`, `generic-webhook.py`

## LoRaWAN (ChirpStack / TTN)

The network server sends hardware **DevEUI**, not your device token. Set DevEUI once:

- On **Create device** (optional LoRa field), or
- **Check connectivity → LoRa** tab on the device.

Copy webhook URL and secret from **Tenant Settings → LoRa & integration webhooks**.

| Network | Endpoint |
|---------|----------|
| ChirpStack | `POST /api/v1/integrations/chirpstack/telemetry?tenantId=...` |
| TTN v3 | `POST /api/v1/integrations/ttn/telemetry?tenantId=...` |

Samples: `examples/integrations/chirpstack-uplink.sample.json`, `ttn-uplink.sample.json`

## WebSocket (device protocol)

Separate from the dashboard Socket.IO client:

1. Connect to `wss://api.autoconnecto.in:{DEVICE_WS_PORT}` (platform default port documented in app **Check connectivity**).
2. Send auth: `{"type":"auth","deviceToken":"..."}`
3. Send telemetry: `{"type":"telemetry","data":{"temperature":24.5}}`

Gateway relay: include `childDeviceId` in the `data` object per platform docs.

## Catalog API

```http
GET https://api.autoconnecto.in/api/v1/connectivity/catalog
```

Returns registered transports and endpoint templates (no auth required for catalog).

## Related docs

- Backend: `backend/docs/CONNECTIVITY.md`, `backend/docs/GATEWAY_DEVICES.md`
- SDK transport topics: `TRANSPORT_ARCHITECTURE.md`
- User docs: [Device connectivity](https://docs.autoconnecto.in/developer/device-connectivity)
