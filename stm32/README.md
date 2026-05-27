# STM32 (Arduino core) — Autoconnecto quick start

This guide helps STM32 users start with Autoconnecto using the **Arduino core** first (fastest path for most people).

> This is **not** a separate SDK. The Autoconnecto MQTT/HTTP contracts are the same across platforms.

## What is supported in this path

- ✅ **MQTT over TLS (MQTTS)** to `mqtt.autoconnecto.in:8883` (recommended)
- ✅ Telemetry, shared attributes, client attributes, RPC (same topics as ESP32 SDK)
- ✅ Uses the same ISRG Root X1+X2 CA bundle (see `raspberrypi/ca/isrg_roots.pem`)

## Requirements

- Arduino IDE 2.x
- STM32 Arduino core installed (commonly **STM32duino / STMicroelectronics** core)
- A board with **enough RAM/flash** for TLS + MQTT
- A network interface:
  - Ethernet (best for industrial), or
  - Wi‑Fi module supported by your core, or
  - Cellular modem (more advanced; not covered in this Arduino-first doc)

## Transport contracts (must match backend)

MQTT topics are **exactly**:

- Telemetry publish:
  - `devices/{deviceToken}/telemetry`
- Client attributes publish:
  - `devices/{deviceToken}/attributes/client`
- Shared attributes subscribe:
  - `devices/{deviceToken}/attributes/shared`
  - `devices/{deviceToken}/attributes/shared/response`
- Shared attributes request publish:
  - `devices/{deviceToken}/attributes/shared/request`
- RPC subscribe:
  - `devices/{deviceToken}/rpc/request/+`
- RPC response publish:
  - `devices/{deviceToken}/rpc/response/{requestId}`

These are the same as the ESP32 Arduino SDK’s internal topic map.

## TLS root certificates

Use the CA bundle at:

- `raspberrypi/ca/isrg_roots.pem`

If your STM32 TLS stack requires a single root, include **both** ISRG roots or the Let’s Encrypt chain may fail for some clients.

## Recommended “first working” flow

1. **Prove TLS** (HTTPS GET to `https://api.autoconnecto.in/healthz`) using your STM32 core’s secure client.
2. **Prove MQTT connect** to `mqtt.autoconnecto.in:8883` with:
   - username = your **device token**
   - clientId = your **device token**
   - CA = ISRG bundle
3. Publish a JSON telemetry payload:

```json
{"temperature": 24.5, "humidity": 61}
```

4. Add subscriptions for shared attributes + RPC and implement the same keys used by ESP32 examples:
   - `channel1..channel4`, `volume`, `limitVoltage1..3`

## Arduino template example

There is a **template sketch** (not part of Arduino Library Manager indexing) here:

- `stm32/arduino/examples/BasicTelemetry_mqtt/BasicTelemetry_mqtt.ino`

You must wire it to your STM32 core’s network + TLS + MQTT libraries (Ethernet/WiFi stack differs per board).

## Arduino-core limitations (important)

STM32 Arduino cores differ widely. Common failure causes:

- **No RTC/time sync** → TLS fails. Ensure SNTP or set a valid time.
- **TLS RAM usage** → you may need larger heap buffers and fewer JSON fields per message.
- **MQTT library choice** → prefer a stable TLS-capable MQTT client for your core.

## Next step (after Arduino core)

For production STM32 devices, most teams eventually move to **CubeIDE + LwIP + mbedTLS** (or Zephyr). When you’re ready, we can add a second `stm32/cubeide/` guide.

