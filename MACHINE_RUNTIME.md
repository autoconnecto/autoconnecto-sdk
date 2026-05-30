# Machine runtime contract

Dashboard widget **`machineFleetRuntime`** and sketch **`examples/Machine_Runtime_mqtt`**.

**Hardware BOM:** [`MACHINE_RUNTIME_HARDWARE_BOM.md`](MACHINE_RUNTIME_HARDWARE_BOM.md) — **Rev 1.0** (pilot / PZEM + 125 kHz) · **Rev 2.0** (new machines: **PN532 NFC**, id+name on card)  
**NFC worker identity (Rev 2):** [`MACHINE_RUNTIME_NFC.md`](MACHINE_RUNTIME_NFC.md)

## Architecture

| Layer | Responsibility |
|-------|----------------|
| **ESP32 + PZEM** | Publish `machine_current_a` (amps RMS) every ~10 s |
| **Platform** | Store owner thresholds (SERVER scope / widget config) |
| **Widget** | Classify **Off** / **Off load (idle)** / **On load**; KPIs, culprits, drawer |

Devices do **not** receive thresholds over MQTT.

## Telemetry (device → platform)

| Key | Required | Description |
|-----|----------|-------------|
| `machine_current_a` | Yes | Current in amps (one phase) |
| `machine_sensor_ok` | No | `false` if sensor/Modbus failed |
| `machine_operator_id` | No | **Rev 2:** `employee_id` read from MIFARE card. **Rev 1:** card UID |
| `machine_operator_name` | No | **Rev 2:** `display_name` read from card (no platform list). **Rev 1:** often same as UID |
| `machine_session_active` | No | `true` between tap IN and tap OUT |

### SHARED attributes (platform → ESP)

Pushed on tool-life save/reset and fetched by the device on **boot**, **MQTT reconnect**, and every **60 s**.

| Key | Description |
|-----|-------------|
| `machine_allow_run` | `false` when tool life expired — SSR interlock opens (machine blocked) |
| `machine_tool_remaining` | Jobs left on current tool |
| `machine_tool_limit` | Max jobs configured |
| `machine_tool_cycles_used` | Jobs consumed on current tool |

Device mirrors session + tool fields to **CLIENT** attributes every **30 s** (and after RFID tap) so the platform survives power cycles.

## Platform thresholds (owner sheet → widget config)

Per machine, in amps:

| Field | Meaning |
|-------|---------|
| **Off A** | No power (~0) |
| **Idle A** | Powered, not working (omit or = Load for 2-level machines) |
| **Load A** | Running / on load |
| **On A** | Same as Load when no separate idle band |

Widget computes cut lines **T1** / **T2** and classifies:

- **Off** — I ≤ T1  
- **Off load** — T1 < I < T2 (3-level)  
- **On load** — I ≥ T2 (or I > T1 for 2-level)

### Job cycles (products)

One **job** = **on-without-load → on-with-load → on-without-load** (see `machineRuntimeCycles.ts`).

### Tool life (consumable tool)

Per machine in widget config:

- **Tool life tracking** on/off  
- **Max jobs** (e.g. 500)  
- Each completed job increments **used**; **remaining** = max − used  
- At **0 remaining**: `machine_tool_life_exhausted`, `machine_allow_run=false`, machine blocked  
- **Admin only**: reset after physical tool replacement  

Widget KPI: **Tool stopped** — count of machines with expired tool life.

## Dashboard widget (`machineFleetRuntime`)

- Full-width default **12×10** grid units  
- KPIs: Total, On load, Off load, Off, Stale, Setup pending, **Tool stopped**  
- **Productivity over time** chart (jobs per bucket + cumulative jobs + on-load hours) — fleet litmus test  
- Main table + alarms rail (desktop) or **Machines / Alarms** tabs (mobile)  
- Tabs: Overview, Culprits, Worst 20 this week  
- Drawer: live amps, **current chart** (T1/T2), thresholds, period metrics, episodes, operator (RFID)  

### Widget configuration (user)

1. **Device type** (required) — e.g. `machines`; all devices with that type appear (1–250+).  
2. **Threshold table**: Off / Idle / Load / On amps → **Save thresholds (platform)**.  
3. **Email report (Brevo)**: recipient list + frequency; see below.

**Runtime (dashboard):** optional **site / area** dropdown filters which machines are shown (KPIs, table, alarms). Reports always use the full device type from config.

## Email reports (Brevo SMTP)

Reports are sent by the backend through the tenant **Brevo SMTP** integration (same transport as OTP / alarm mail).

| Action in widget config | Effect |
|-------------------------|--------|
| **Send test email** | One-off HTML report; rolling window (e.g. last 7 days for weekly). |
| **Save & send report** | Registers schedule + sends immediately to all listed emails. |
| **Send now** | Resend using saved schedule (rolling window). |

**Scheduled delivery:** cron runs **hourly (IST)**; when due (daily / weekly / monthly / yearly), sends to all addresses on the schedule.

**Report content:** HTML table — machine name, uptime (on load), runtime, load drops, downtime — ranked by downtime. Uses `machine_current_a` history and per-device thresholds from the schedule / SERVER attrs.

**Backend env (admin):** `BREVO_SMTP_USER`, `BREVO_SMTP_PASSWORD`, `MAIL_FROM_EMAIL` (optional `BREVO_SMTP_HOST`, `BREVO_SMTP_PORT`).

## APIs

| Method | Path | Purpose |
|--------|------|---------|
| `POST` | `/api/machine-fleet/range-summary` | Batch period metrics |
| `POST` | `/api/machine-fleet/device-detail` | Single machine period metrics |
| `POST` | `/api/machine-fleet/productivity-series` | Time-bucketed jobs + on-load minutes for productivity chart |
| `POST` | `/api/machine-fleet/cycles/sync` | Detect jobs from telemetry → decrement tool life |
| `POST` | `/api/machine-fleet/tool-life/reset` | Admin reset tool counter |
| `POST` | `/api/machine-fleet/tool-life/batch` | Load tool life state |
| `POST` | `/api/machine-fleet/reports/register` | Save schedule (`sendNow` optional) |
| `POST` | `/api/machine-fleet/reports/test` | Test send to widget emails |
| `POST` | `/api/machine-fleet/reports/send-now` | Send saved schedule now |
| `DELETE` | `/api/machine-fleet/reports/:scheduleId` | Remove schedule |

Schedules are stored per tenant in attribute `machine_fleet_report_schedules`.

## Firmware

| Rev | Sketch | RFID |
|-----|--------|------|
| 1.0 | [`examples/Machine_Runtime_mqtt/`](examples/Machine_Runtime_mqtt/) | 125 kHz EM4100 UART |
| **2.0** | [`examples/Machine_Runtime_NFC_mqtt/`](examples/Machine_Runtime_NFC_mqtt/) | **PN532 I2C** — see [`MACHINE_RUNTIME_NFC.md`](MACHINE_RUNTIME_NFC.md) |

PZEM on UART2 (GPIO 16/17); `machine_current_a` ~10 s. Set `PZEM_DEMO_FALLBACK` to `0` in production.

**Rev 2 reader (frozen):** [Mifra PN532 MT0359](https://mifraelectronics.com/product/pn532-nfc-rfid-read-write-module/).
