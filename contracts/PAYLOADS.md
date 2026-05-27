# Payload contract (telemetry, attributes, RPC)

This document defines **payload shapes** expected by the Autoconnecto backend.

## Telemetry (MQTT)

Topic: `devices/{token}/telemetry`

- Payload: JSON object
- Keys: arbitrary, but examples standardize around:
  - `temperature`, `humidity`, `current`, `power`, `voltage1..3`
  - Generator demo: `gen_*` keys (see `examples/Generator_Monitoring_mqtt`)

## Client attributes (MQTT)

Topic: `devices/{token}/attributes/client`

- Payload: JSON object
- Values are stored as the device’s latest client state (used for dashboards and confirmation)

## Shared attributes (MQTT)

Topic (platform → device): `devices/{token}/attributes/shared`

- Payload: JSON object (flat)
- Devices should:
  - apply values to hardware
  - **confirm** via client attributes with the same key(s)

### Shared attribute request

Topic (device → platform): `devices/{token}/attributes/shared/request`

Payload:

```json
{ "requestId": 123456, "keys": "channel1,volume" }
```

Platform responds on: `devices/{token}/attributes/shared/response`

## RPC

RPC request topic: `devices/{token}/rpc/request/{requestId}`

Example:

```json
{
  "method": "getStatus",
  "params": { "count": 5 },
  "requestId": "1716720000000"
}
```

RPC response topic: `devices/{token}/rpc/response/{requestId}`

Response payload is device-defined, but should include at least:

```json
{ "success": true }
```

## HTTP device-token API (reference)

The device-token HTTP ingestion endpoints are described in `CONNECTIVITY.md`.

