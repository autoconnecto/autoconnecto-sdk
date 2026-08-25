---
title: Generator Monitoring
description: Diesel generator fleet dashboards with gen_* telemetry, alarms, and SDK examples.
---

# Generator Monitoring

Dashboard widget: **`generatorMonitoring`**

Monitor **diesel generator (DG) sets** — run hours, electrical output, fuel level, RPM, and fault states. Ideal for facilities, hospitals, malls, telecom sites, and DG AMC vendors.

## Telemetry contract (`gen_*` keys)

The ESP32 example [`Generator_Monitoring_mqtt`](https://github.com/autoconnecto/autoconnecto-sdk/tree/main/examples/Generator_Monitoring_mqtt) publishes keys such as:

| Key | Description |
|-----|-------------|
| `gen_run_state` | stopped / starting / running / fault |
| `gen_kw` | Active power (kW) |
| `gen_kva` | Apparent power (kVA) |
| `gen_hz` | Frequency |
| `gen_v_l1`, `gen_v_l2`, `gen_v_l3` | Phase voltages |
| `gen_rpm` | Engine RPM |
| `gen_fuel_level_pct` | Fuel tank level |
| `gen_alarm_count` | Active alarm count |

The sketch simulates realistic DG cycles for demos. Production deployments read Modbus from generator controllers (Deep Sea, ComAp, SmartGen) or analog sensors.

## Connectivity

| Path | Example |
|------|---------|
| ESP32 MQTT | `examples/Generator_Monitoring_mqtt` |
| Raspberry Pi Python | `raspberrypi/examples/Generator_Monitoring_mqtt` |
| LTE remote sites | `BasicTelemetry_lte_ppp_mqtt` + generator keys |
| Modbus gateway | Poll controller registers → MQTTS telemetry |

## Dashboard setup

1. Create a device and assign to your asset tree (SITE → AREA → ZONE).
2. Add the **Generator Monitoring** widget; map `gen_*` keys automatically via widget type.
3. Configure alarms on `gen_run_state`, low fuel, or fault conditions.

## Use cases

- **DG AMC contracts** — run-hour tracking and service interval alerts
- **Fuel theft / low fuel** — threshold alarms and email escalation
- **Multi-DG sites** — one dashboard per facility or fleet rollup
- **Grid vs DG** — correlate with utility meter telemetry on same platform

## Related docs

- [Device connectivity](https://docs.autoconnecto.in/developer/device-connectivity)
- [CONNECTIVITY.md](https://github.com/autoconnecto/autoconnecto-sdk/blob/main/CONNECTIVITY.md) (SDK repo)
