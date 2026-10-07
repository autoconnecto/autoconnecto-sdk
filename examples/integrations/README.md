# Integration webhook examples

These samples are for **servers, scripts, LoRa network servers, and Linux gateways** — not Arduino sketches.

Arduino companion sketches:

- `../GatewayRelay_http` — ESP32 gateway hub → child telemetry
- `../TelemetryBatch_http` — buffered HTTP batch upload
- `../BasicTelemetry_http` — single-device HTTPS telemetry

Copy `env.example` values from:

- **Integrations** (`/integrations`) — create/start an integration, copy HTTP endpoint + generate webhook secret
- **Device Details → Check connectivity** — device token, device id, DevEUI

Legacy type URLs (`/api/v1/integrations/{chirpstack|ttn|generic}/telemetry`) still work and resolve the tenant’s default active integration of that type when one exists. Named Hub URLs use `/api/v1/integrations/i/{integrationId}/telemetry`.

## Generic webhook

Forward any JSON telemetry from your system to Autoconnecto.

| File | Use |
|------|-----|
| `generic-webhook.sh` | curl (Linux/macOS/Git Bash) |
| `generic-webhook.ps1` | PowerShell |
| `generic-webhook.py` | Python 3 |

```bash
export API_BASE=https://api.autoconnecto.in
export TENANT_ID=your-tenant-uuid
export WEBHOOK_SECRET=your-secret
export DEVICE_TOKEN=your-device-token
# optional instead of token:
# export DEVICE_ID=87533b15-c04a-4c27-bb2a-adcdaf2444f4

./generic-webhook.sh
```

## ChirpStack

1. Create a **ChirpStack** integration in Autoconnecto and start it.
2. Set DevEUI on the device (`lorawan.dev_eui` / `integration.lorawan.dev_eui`).
3. In ChirpStack: **Applications → Integrations → HTTP** → paste the integration endpoint.
4. Header: `X-Webhook-Secret: {secret}`

Test locally:

```bash
./chirpstack-webhook.sh
```

Payload shape: `chirpstack-uplink.sample.json`.

## TTN / The Things Stack v3

Same DevEUI attribute as ChirpStack. Create a **TTN** integration, paste the webhook URL into Things Stack, then:

```bash
./ttn-webhook.sh
```

## Gateway relay

Two supported paths:

1. **Device-token HTTP** (preferred for edge gateways) — matches `GatewayRelay_http.ino`:

```bash
export GATEWAY_TOKEN=your-gateway-device-token
export CHILD_DEVICE_ID=your-child-device-uuid
./gateway-relay-webhook.sh
```

2. **Generic integration webhook** — set `MODE=webhook` plus `TENANT_ID` / `WEBHOOK_SECRET`.

PowerShell: `gateway-relay-webhook.ps1`.

For Modbus/DTU **Solutions** stacks (EnergyFleet / ClimateFleet P1), see platform **Solutions** and docs `solutions/connectivity-profiles`, `solutions/energy-fleet`, `solutions/climate-fleet`.

## OPC‑UA / Modbus / MQTT bridge (Linux SBC)

| Folder | Role |
|--------|------|
| `opcua-gateway/` | OPC‑UA → Autoconnecto MQTTS |
| `modbus-gateway/` | Modbus TCP/RTU → Autoconnecto MQTTS |
| `mqtt-bridge/` | External broker → Autoconnecto token topics |

## See also

- [`../../CONNECTIVITY.md`](../../CONNECTIVITY.md) — SDK connectivity guide
- Public docs: [Integrations overview](https://docs.autoconnecto.in/integrations/integrations-overview)
- Backend reference: `backend/docs/CONNECTIVITY.md` (in the Autoconnecto workspace)
