# OPC‑UA gateway → Autoconnecto (MQTT)

This example reads values from an OPC‑UA server and forwards them to Autoconnecto as **device telemetry** over MQTT.

It is intended for **Linux SBC / gateways** (Raspberry Pi, industrial PCs).

## Requirements

- Python 3.9+
- OPC‑UA endpoint URL
- Autoconnecto device token

## Install

```bash
cd examples/integrations/opcua-gateway
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Configure

Copy the sample config and edit:

```bash
cp config.example.json config.json
```

Fields:

- `opcua.endpoint`: e.g. `opc.tcp://192.168.1.10:4840`
- `opcua.username/password`: optional
- `autoconnecto.deviceToken`: device token in Autoconnecto
- `autoconnecto.mqttHost/mqttPort`: defaults ok for production
- `mapping[]`: list of OPC‑UA nodeIds mapped to telemetry keys

## Run

```bash
python opcua_gateway_mqtt.py config.json
```

## Notes

- Telemetry is published to: `devices/{token}/telemetry` (same as ESP32 SDK).
- Data types are coerced to JSON‑safe types (bool/int/float/string).
- If a read fails, the script reconnects and continues.

