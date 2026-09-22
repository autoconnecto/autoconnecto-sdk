---
title: Edge gateways
description: Modbus, OPC-UA, MQTT bridge, and LoRaWAN integrations for brownfield industrial equipment.
---

# Edge gateways & integrations

Connect **legacy PLCs, meters, and LoRa sensors** to Autoconnecto without replacing existing automation. Gateway scripts run on a Linux SBC (Raspberry Pi recommended) or any server with network access to your plant.

## Integration paths

| Source | SDK sample | Auth |
|--------|------------|------|
| Modbus TCP/RTU | `examples/integrations/modbus-gateway/` | Device token (MQTTS) |
| OPC-UA server | `examples/integrations/opcua-gateway/` | Device token (MQTTS) |
| Third-party MQTT | `examples/integrations/mqtt-bridge/` | Device token (MQTTS) |
| Generic JSON webhook | `examples/integrations/generic-webhook.*` | Tenant webhook secret |
| ChirpStack LoRaWAN | `examples/integrations/chirpstack-*` | Webhook secret + DevEUI |
| TTN / The Things Stack v3 | `examples/integrations/ttn-*` | Webhook secret + DevEUI |

## Gateway child-device relay

For a **hub device** that aggregates multiple endpoints:

1. Mark parent device as **Gateway** in the platform.
2. Create **child** devices with parent set to the gateway.
3. Publish using the **gateway token** and include `childDeviceId`:

```json
{
  "deviceToken": "gateway-access-token",
  "childDeviceId": "child-device-uuid",
  "telemetry": { "temperature": 24.1 }
}
```

Example: `examples/GatewayRelay_http`

## Raspberry Pi deployment

Python examples under [`raspberrypi/examples/`](https://github.com/autoconnecto/autoconnecto-sdk/tree/main/raspberrypi/examples) mirror ESP32 sketch names. A **systemd unit template** is included for run-on-boot gateway services.

## Typical architecture

```
PLC / meter (Modbus)  ──►  Pi gateway  ──►  MQTTS  ──►  Autoconnecto
OPC-UA server         ──►  opcua_gateway_mqtt.py
LoRa sensor           ──►  ChirpStack  ──►  webhook  ──►  Autoconnecto
```

## Tenant configuration

- **Device Details → Check connectivity** — per-device curl/MQTT samples
- **Tenant Settings → LoRa & integration webhooks** — webhook URLs and secret header

Full guide: [CONNECTIVITY.md](https://github.com/autoconnecto/autoconnecto-sdk/blob/main/CONNECTIVITY.md)
