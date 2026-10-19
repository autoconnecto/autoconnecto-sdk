# SDK Transport Architecture (Arduino/ESP32)

This document describes the device SDK transport implementation under `sdk/src`.

## CoAP and AMQP

The Arduino SDK stays on MQTT. CoAP and AMQP are additional ingest paths on the API. They accept the same telemetry JSON object and the same device token. They do not replace `MQTTTransport`.

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
- `SDKConfig.wssPort` defaults to **8084**. `allowInsecureTLS` defaults to false. `rootCA` defaults to `AUTOCONNECTO_ROOT_CA`.
- A top-level numeric `ts` on telemetry is the reading time. The API does not store it as a key.

### Authentication

- MQTT username and client_id are both set to the device token. A rotated token is refused at connect. The payload is JSON.

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

- `devices/{deviceToken}/rpc/response/{requestId}`

## Backend alignment (confirmed)

The API reads command replies on:

- `devices/+/rpc/response/+`
- `v1/devices/+/rpc/response/+`

`v1/devices/me/rpc/response/+` is ignored. That topic does not carry the device token.

## Global singleton constraint (confirmed)

`MQTTTransport.cpp` uses a file-scope global pointer (`globalTransportInstance`) to route static MQTT callbacks to the instance method.

Implication:
- The implementation assumes a single active `MQTTTransport` instance per process.

## Related examples (SDK repo)

| Path | Purpose |
|------|---------|
| `examples/BasicTelemetry_mqtt` | MQTT telemetry |
| `examples/Generator_Monitoring_mqtt` | MQTT `gen_*` generator telemetry |
| `examples/BasicTelemetry_http` | HTTPS telemetry |
| `examples/TelemetryBatch_http` | `POST .../telemetry/batch` |
| `examples/GatewayRelay_http` | Gateway `childDeviceId` relay over HTTPS |
| `examples/integrations/` | Generic / ChirpStack / TTN webhook scripts |

See `CONNECTIVITY.md` for full ingress documentation.

## Current Status

- MQTT-over-WSS is the implemented transport path.
- Telemetry, attributes, and RPC request handling are present.

## Next Priorities (recommended)

- Establish and enforce a single authoritative topic contract shared between backend and SDK.
- If multiple transports or multiple SDK instances are ever required, remove the global singleton dispatch pattern.

