# Device Lifecycle (SDK ↔ Backend)

This document describes the end-to-end device lifecycle implied by the current SDK and backend code.

## Confirmed SDK lifecycle

### 1) Configure

Device code provides `SDKConfig` including:
- WiFi credentials
- MQTT host/ports (WSS port used by current transport)
- `deviceToken`
- TLS configuration (`rootCA` and `allowInsecureTLS`)

`rootCA` accepts a **multi-cert PEM bundle**. The example sketches ship with both `ISRG Root X1` (RSA, valid → 2035) and `ISRG Root X2` (ECDSA, valid → 2040) concatenated in one string. mbedtls (via esp_tls) validates the broker chain if it terminates at **any** root in the bundle. This keeps existing devices working across LE's gradual X1 → X2 transition without OTA firmware updates.

### 2) Start

`AutoconnectoSDK::begin(config)`:
- stores config
- connects WiFi (blocking loop until connected)
- configures `ConnectionManager`
- starts transport via `ConnectionManager.begin()`

### 3) Run loop

`AutoconnectoSDK::loop()`:
- calls `manager.loop()` to drive reconnect logic
- derives connection state from `activeTransport() != nullptr`
- triggers `onConnect` / `onDisconnect` callbacks on state transitions

## Confirmed device → cloud actions

### Telemetry

SDK publishes telemetry to:
- `devices/{deviceToken}/telemetry`

Backend consumes:
- `devices/+/telemetry`
and ingests into telemetry pipeline, then emits Socket.IO realtime events to the UI.

### Client attributes

SDK publishes client attributes to:
- `devices/{deviceToken}/attributes/client`

Backend consumes:
- `devices/+/attributes/client`
and persists to DB (and may emit updates to UI depending on flow).

## Confirmed cloud → device actions

### Shared attributes

Backend publishes retained shared attribute snapshots to:
- `devices/{token}/attributes/shared`

SDK subscribes to:
- `devices/{deviceToken}/attributes/shared`
- `devices/{deviceToken}/attributes/shared/response`

SDK requests shared attributes by publishing to:
- `devices/{deviceToken}/attributes/shared/request`

Backend listens to shared attribute requests and publishes response to:
- `devices/{token}/attributes/shared/response`

## RPC / ACK (confirmed behavior, but contract alignment required)

SDK subscribes to:
- `devices/{deviceToken}/rpc/request/+`

SDK replies by publishing to:
- `v1/devices/me/rpc/response/{requestId}`

Backend consumes RPC responses from:
- `devices/+/rpc/response/+`

