# SDK Transport Architecture (Arduino/ESP32)

This document describes the device SDK transport implementation under `sdk/src`.

## Confirmed transport selection

`src/core/ConnectionManager.h`:
- `ConnectionManager` owns a single `MQTTTransport mqtt;`
- `activeTransport()` returns the currently active transport (or null).

This implies the SDK is currently designed around a single active transport instance.

## MQTT transport (confirmed)

`src/transport/MQTTTransport.*`:

### Broker URI and scheme

- `buildBrokerURI()` sets `usingWSS = true` and returns:
  - `wss://{mqttHost}:{wssPort}/mqtt`

### Authentication

- MQTT username and client_id are both set to the device token.

### Subscribe topics (on connect)

On `MQTT_EVENT_CONNECTED`, the SDK subscribes to:

- `devices/{deviceToken}/attributes/shared`
- `devices/{deviceToken}/attributes/shared/response`
- `devices/{deviceToken}/rpc/request/+`

It also performs an initial shared attribute request.

### Publish topics (confirmed)

- Telemetry: `devices/{deviceToken}/telemetry`
- Client attributes: `devices/{deviceToken}/attributes/client`
- Shared attribute request: `devices/{deviceToken}/attributes/shared/request`

### RPC response topic (confirmed in SDK code)

SDK publishes RPC responses to:

- `v1/devices/me/rpc/response/{requestId}`

This must be reconciled with backend subscriptions (see next section).

## Backend alignment risks (confirmed)

The backend includes an MQTT consumer subscribing to:
- `devices/+/rpc/response/+`

If devices publish RPC responses only to `v1/devices/me/rpc/response/+`, the backend will not observe those messages unless:
- the backend also subscribes to `v1/devices/me/rpc/response/+`, or
- the SDK publishes to `devices/{token}/rpc/response/{id}`, or
- there is broker-side bridging.

## Global singleton constraint (confirmed)

`MQTTTransport.cpp` uses a file-scope global pointer (`globalTransportInstance`) to route static MQTT callbacks to the instance method.

Implication:
- The implementation assumes a single active `MQTTTransport` instance per process.

## Current Status

- MQTT-over-WSS is the implemented transport path.
- Telemetry, attributes, and RPC request handling are present.

## Next Priorities (recommended)

- Establish and enforce a single authoritative topic contract shared between backend and SDK.
- If multiple transports or multiple SDK instances are ever required, remove the global singleton dispatch pattern.

