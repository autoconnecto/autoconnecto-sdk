---
title: What's new
description: Latest Autoconnecto platform and SDK developments — vertical widgets, OTA, edge gateways, and expanded device targets.
---

# What's new

Recent platform and SDK releases add **vertical industry widgets**, **OTA firmware**, **edge gateway integrations**, and broader hardware support beyond ESP32-only deployments.

## Vertical solution widgets

### Machine Fleet Runtime (`machineFleetRuntime`)

Factory machine monitoring from current draw:

- Classify **Off**, **Off load (idle)**, and **On load** from `machine_current_a`
- Productivity charts, downtime culprits, and worst-20 reports
- **Tool life tracking** with SSR interlock when consumable life expires
- Worker sessions via **125 kHz RFID**, **PN532 NFC**, or **BLE Android app** (Rev 3)
- **Scheduled email reports** via tenant Brevo SMTP integration
- Frozen hardware BOM (~₹2,400–3,200 per machine) — ESP32 + PZEM-004T + Fotek SSR

→ [Machine Fleet solution guide](../solutions/machine-fleet.md) · SDK: [MACHINE_RUNTIME.md](https://github.com/autoconnecto/autoconnecto-sdk/blob/main/MACHINE_RUNTIME.md)

### Generator Monitoring (`generatorMonitoring`)

Diesel generator fleet dashboards:

- `gen_*` telemetry contract (run state, kW, kVA, RPM, fuel, alarms)
- ESP32 MQTT example and Raspberry Pi Python mirror
- LTE PPP option for sites without reliable WiFi

→ [Generator Monitoring guide](../solutions/generator-monitoring.md)

## Edge gateways & brownfield

Connect existing plant equipment without replacing PLCs:

- **Modbus TCP/RTU** gateway (Linux SBC)
- **OPC-UA** gateway
- **MQTT bridge** (third-party broker → Autoconnecto token topics)
- **Gateway child-device relay** (one gateway token, many child devices)
- LoRaWAN via **ChirpStack** or **TTN / The Things Stack v3** webhooks

→ [Edge gateways guide](../solutions/edge-gateways.md)

## SDK & connectivity (v1.3.7)

| Feature | Details |
|---------|---------|
| **OTA firmware** | ThingsBoard-style shared attributes + chunked HTTPS download |
| **LTE PPP** | Quectel EC200 on ESP32 core 3.x (`BasicTelemetry_lte_ppp_mqtt`) |
| **Batch telemetry** | HTTP batch (100 items) and MQTT buffered flush examples |
| **Watchdog reconnect** | Long-disconnect recovery pattern |
| **Raspberry Pi** | Python examples mirroring ESP32 names + systemd unit template |
| **STM32** | Arduino-core quick start + CubeIDE scaffold docs |
| **Transport contracts** | Authoritative `contracts/MQTT_TOPICS.md` and `contracts/PAYLOADS.md` |

→ [OTA firmware](../developer/ota-firmware.md) · [Device connectivity](https://docs.autoconnecto.in/developer/device-connectivity)

## Platform (unchanged strengths)

- **Reboot-safe control** — shared attributes + client confirmation after power cycle
- **Optional data & attribute pipelines** per device profile
- **Asset hierarchy** — SITE → AREA → ZONE for scoping dashboards and widgets
- **Multi-tenant SaaS** with INR plans and white-label on Growth/Enterprise

## Mobile companion

- **Android APK** — live telemetry and alarms (same Cognito login as web app)
- **Machine worker app (Rev 3)** — BLE GATT worker identity for machine fleet (see SDK `MACHINE_RUNTIME_BLE.md`)

Download: [autoconnecto-mobile releases](https://github.com/autoconnecto/autoconnecto-mobile/releases/latest)
