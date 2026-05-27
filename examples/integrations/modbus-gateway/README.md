# Modbus gateway → Autoconnecto (MQTT)

This example polls Modbus registers (TCP or RTU) and forwards values to Autoconnecto as **device telemetry**.

It is intended for **Linux SBC / gateways**.

## Install

```bash
cd examples/integrations/modbus-gateway
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Configure

```bash
cp config.example.json config.json
python modbus_gateway_mqtt.py config.json
```

## Mapping

Each mapping item reads one Modbus register and writes it to a telemetry key.

Common patterns:
- scale (e.g. raw 253 → 25.3°C)
- signed vs unsigned

