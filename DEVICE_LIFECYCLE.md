# Device Lifecycle (SDK ↔ Backend)

This document describes the end-to-end device lifecycle implied by the current SDK and backend code.

## Confirmed SDK lifecycle

### 1) Configure

Device code provides `SDKConfig` including:
- **Network:** `networkMode` defaults to **WiFi** (existing behaviour). Optional **LTE PPP** (`NetworkMode::LtePpp`, Quectel EC200) — compile with `-DAUTOCONNECTO_ENABLE_LTE_PPP=1` and ESP32 core 3.x PPP library. See `LTE_PPP_EC200.md`.
- WiFi credentials (when `networkMode` is WiFi)
- MQTT host/ports (WSS port used by current transport)
- `deviceToken`
- TLS configuration (`rootCA` and `allowInsecureTLS`)

`rootCA` accepts a **multi-cert PEM bundle**. The canonical bundle is `AUTOCONNECTO_ROOT_CA` in `src/AutoconnectoIsrgRoots.h` (included by `AutoconnectoSDK.h` and by HTTPS-only example sketches). It concatenates `ISRG Root X1` (RSA, valid → 2035) and `ISRG Root X2` (ECDSA, valid → 2040). mbedtls (via esp_tls) validates the broker chain if it terminates at **any** root in the bundle. This keeps existing devices working across LE's gradual X1 → X2 transition without OTA firmware updates.

### 2) Start

`AutoconnectoSDK::begin(config)`:
- stores config
- connects network: **WiFi** (default) or **LTE PPP** when configured
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

## RPC / ACK (two-way)

SDK subscribes to:
- `devices/{deviceToken}/rpc/request/+`

SDK replies by publishing to:
- `devices/{deviceToken}/rpc/response/{requestId}`

(use the same `requestId` from the request topic suffix)

Backend consumes RPC responses from:
- `devices/+/rpc/response/+`
(and legacy `v1/devices/+/rpc/response/+`)

Dashboard Toggle / GPIO / RPC widgets wait on Socket.IO `device_rpc_response` matching that `requestId` before showing Confirmed.

## Shared-attribute control loop (preferred for setpoints)

1. Platform writes SHARED (desired).
2. Device receives retained SHARED (or pulls on connect).
3. Device applies hardware, then publishes CLIENT with the same keys.
4. Dashboard shows Desired (SHARED) vs Device (CLIENT).

See `examples/SwitchControl_mqtt` for the apply + `sendClientAttribute` pattern.

