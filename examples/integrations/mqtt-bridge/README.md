# MQTT bridge → Autoconnecto (token topics)

This gateway subscribes to a third-party MQTT broker/topic and republishes messages into Autoconnecto using a **device token**.

Use cases:
- Existing PLC gateway already publishes MQTT (local broker)
- You want to forward selected keys to Autoconnecto dashboards/alarms

## Install

```bash
cd examples/integrations/mqtt-bridge
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## Configure

```bash
cp config.example.json config.json
python mqtt_bridge.py config.json
```

## Contract

Autoconnecto publish topic:

- `devices/{token}/telemetry`

Payload must be a JSON object (flat). The bridge can either:
- forward a JSON object directly, or
- wrap a scalar under a key

