# nRF52 (Zephyr) — Autoconnecto quick start

This is a **docs-only** guide for teams building BLE-centric devices on nRF52 that need cloud telemetry/control.

Recommended stack: **Zephyr RTOS** (networking + TLS + MQTT are first-class).

## Why Zephyr for nRF52

- Consistent MQTT + TLS APIs
- Better portability than ad-hoc Arduino cores

## What to implement

1. TLS root trust:
   - Use `../..//raspberrypi/ca/isrg_roots.pem` as the CA bundle (convert to Zephyr credential storage format if needed).
2. MQTT connect:
   - host: `mqtt.autoconnecto.in`
   - port: `8883`
   - username: `{deviceToken}`
   - clientId: `{deviceToken}`
3. Publish telemetry:
   - topic: `devices/{token}/telemetry`
   - JSON payload (flat)
4. Subscribe shared attributes:
   - `devices/{token}/attributes/shared`
   - confirm via `devices/{token}/attributes/client`

## Contracts

- `../../contracts/MQTT_TOPICS.md`
- `../../contracts/PAYLOADS.md`

