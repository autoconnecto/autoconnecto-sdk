---
title: Machine Fleet Runtime
description: Factory machine monitoring widget — uptime, idle time, tool life, worker sessions, and email reports.
---

# Machine Fleet Runtime

Dashboard widget: **`machineFleetRuntime`**

Monitor discrete manufacturing machines from **current draw** — no PLC replacement required. The platform classifies each machine as **Off**, **Off load (idle)**, or **On load** using thresholds you configure per device type.

## What you get

| Capability | Description |
|------------|-------------|
| **Fleet KPIs** | Total, on load, off load, off, unavailable, tool stopped |
| **Productivity chart** | Jobs per time bucket + cumulative jobs + on-load hours |
| **Culprits & worst 20** | Which machines drove downtime this week |
| **Operator sessions** | RFID tap IN/OUT, NFC card, or BLE worker app |
| **Tool life** | Job counter on consumable tools; SSR blocks machine at limit |
| **Email reports** | Daily/weekly/monthly HTML summaries via Brevo SMTP |

## Telemetry contract (device → platform)

| Key | Required | Description |
|-----|----------|-------------|
| `machine_current_a` | Yes | Current in amps (PZEM-004T) |
| `machine_voltage_v` | No | Line voltage |
| `machine_power_w` | No | Active power |
| `machine_operator_id` | No | Worker ID (RFID UID, NFC `employee_id`, or BLE) |
| `machine_operator_name` | No | Display name from NFC/BLE |
| `machine_session_active` | No | Session in progress |
| `machine_session_start_ts` | No | Unix seconds — session start |
| `machine_session_end_ts` | No | Unix seconds — session end |

Shared attributes (platform → device): `machine_allow_run`, `machine_tool_remaining`, `machine_tool_limit`, `machine_tool_cycles_used`.

## Hardware (Rev 1.0 BOM)

Per machine (~₹2,770 typical):

- ESP32 DevKit (WROOM-32)
- PZEM-004T (100 A, Modbus RTU)
- MAX485 + 125 kHz RFID reader (Rev 1) or PN532 NFC (Rev 2)
- Fotek SSR on enable/coil path (not motor power)
- IP65 enclosure + 5 V PSU

Full BOM: [MACHINE_RUNTIME_HARDWARE_BOM.md](https://github.com/autoconnecto/autoconnecto-sdk/blob/main/MACHINE_RUNTIME_HARDWARE_BOM.md)

## Firmware & setup

1. Flash [`Machine_Runtime_mqtt`](https://github.com/autoconnecto/autoconnecto-sdk/tree/main/examples/Machine_Runtime_mqtt) on ESP32.
2. Create devices with type e.g. `machines`; assign to **ZONE** under SITE/AREA assets.
3. Add **Machine Fleet Runtime** widget to dashboard; set Off/Idle/Load amp thresholds.
4. Optional: configure **Email report (Brevo)** in widget config.

## Worker identity evolution

| Rev | Method | Notes |
|-----|--------|-------|
| 1.0 | 125 kHz RFID | Card UID only |
| 2.0 | PN532 NFC | `employee_id` + `display_name` on MIFARE card |
| 3.0 | BLE Android app | GATT service; no RFID pod at machine |

## API endpoints

- `POST /api/machine-fleet/range-summary`
- `POST /api/machine-fleet/productivity-series`
- `POST /api/machine-fleet/reports/register`

Full contract: [MACHINE_RUNTIME.md](https://github.com/autoconnecto/autoconnecto-sdk/blob/main/MACHINE_RUNTIME.md)
