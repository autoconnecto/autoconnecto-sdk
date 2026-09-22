# STM32CubeIDE path (production STM32)

This guide is for teams shipping STM32 devices in production using:

- STM32CubeIDE / CubeMX
- LwIP (Ethernet / PPP / Wi‑Fi module integration)
- mbedTLS
- an embedded MQTT client

It is intentionally separate from the Arduino-core quick start (`../README.md`).

## Scope (v1)

- Prove TLS (HTTPS GET `/healthz`)
- Prove MQTTS connect to `mqtt.autoconnecto.in:8883` (token as username + clientId)
- Publish telemetry payload to `devices/{token}/telemetry`

## Key references

- MQTT topic contract: `../../contracts/MQTT_TOPICS.md`
- Payload contract: `../../contracts/PAYLOADS.md`
- TLS CA bundle: `../../raspberrypi/ca/isrg_roots.pem`

## Planned content

- Ethernet reference (recommended industrial baseline)
- Cellular PPP reference (optional)
- Shared attributes subscribe + confirmation loop
- RPC request/response

