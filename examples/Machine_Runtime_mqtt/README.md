# Machine_Runtime_mqtt

ESP32 example for the **Machine Fleet** dashboard widget (`machineFleetRuntime`).

## Quick start

1. Create a device in Autoconnecto and copy the **device token**.
2. Assign the device to a **ZONE** asset (under `SITE → AREA → ZONE`).
3. Edit `Machine_Runtime_mqtt.ino`: Wi‑Fi, `mqttHost`, `deviceToken`.
4. Flash and open Serial Monitor — you should see `[TEL] machine_current_a=…`.
5. On a dashboard, add **Machine Fleet**, set **Root asset**, enter **thresholds** per machine, and **Save thresholds (platform)**.

## Payload contract

| Type | Key | Notes |
|------|-----|--------|
| Telemetry (required) | `machine_current_a` | Amps RMS (one phase) |
| Telemetry (optional) | `machine_sensor_ok` | `false` if Modbus/ADC failed |

**Thresholds (Off / Idle / Load amps)** are configured in the dashboard and stored on the platform — not on the ESP.

Full details: [`../../MACHINE_RUNTIME.md`](../../MACHINE_RUNTIME.md).

## Production notes

- Use **PZEM-004T** (or similar) on the same phase that powers the ESP if applicable.
- Publish every **10 s**; the widget uses stale window (default 3 min) for link state.
- Sketch includes PZEM Modbus RTU on UART2 (GPIO 16/17). Set `PZEM_DEMO_FALLBACK` to `0` in production so failed reads do not simulate amps.

## Dashboard reports

In the widget config panel, use **Email report (Brevo)** to send factory summary emails (uptime, downtime, culprits table). Requires platform Brevo SMTP. See [`../../MACHINE_RUNTIME.md`](../../MACHINE_RUNTIME.md#email-reports-brevo-smtp).
