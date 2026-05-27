# MQTT topic contract (device ↔ platform)

This is the **authoritative** topic contract for Autoconnecto devices and gateways.

## Naming

- Device room (frontend/backend realtime): `device:{deviceId}` (unrelated to MQTT topic names)
- MQTT topics use the **device token** in the topic path.

## Authentication

- **Username**: device token
- **Client ID**: device token
- Password: unused (blank)

## Topics (device token)

Assume `token = {deviceToken}`.

### Device → Platform

- **Telemetry**
  - `devices/{token}/telemetry`
  - Payload: JSON object (flat)

- **Client attributes**
  - `devices/{token}/attributes/client`
  - Payload: JSON object (flat)

- **Shared attributes request**
  - `devices/{token}/attributes/shared/request`
  - Payload: JSON object with:
    - `requestId` (string or number)
    - optional `keys` (string; comma-separated or JSON-encoded list depending on device implementation)

- **RPC response**
  - `devices/{token}/rpc/response/{requestId}`
  - Payload: JSON object

### Platform → Device

- **Shared attributes (retained snapshot)**
  - `devices/{token}/attributes/shared`
  - Payload: JSON object (flat key/value pairs)

- **Shared attributes response**
  - `devices/{token}/attributes/shared/response`
  - Payload: JSON object (flat key/value pairs)

- **RPC request**
  - `devices/{token}/rpc/request/{requestId}`
  - Payload: JSON object with:
    - `method` (string)
    - `params` (object)
    - `requestId` (string/number; should match topic suffix)

## QoS / retain

Baseline recommendations (devices may deviate; document if you do):

- Telemetry: QoS 1, retain 0
- Client attributes: QoS 1, retain 0
- Shared attributes: platform publishes retained snapshots (retain 1)
- RPC: QoS 1, retain 0

