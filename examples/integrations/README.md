# Integration webhook examples

These samples are for **servers, scripts, and LoRa network configuration** — not Arduino sketches.

Copy `env.example` to `.env` and fill in values from:

- **Tenant Settings → LoRa & integration webhooks** (URLs + generate secret)
- **Device Details → Check connectivity** (device token, device id)

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

1. Create device in Autoconnecto; set **DevEUI** on create or in Check connectivity → LoRa.
2. In ChirpStack: **Applications → Integrations → HTTP** → paste tenant ChirpStack URL from Tenant Settings.
3. Header: `X-Webhook-Secret: {secret}`

Test locally with the sample uplink body:

```bash
./chirpstack-webhook.sh
```

Payload shape: `chirpstack-uplink.sample.json` (maps `deviceInfo.devEui` → your device).

## TTN / The Things Stack v3

1. Set DevEUI on the Autoconnecto device (same as ChirpStack).
2. TTN webhook URL from Tenant Settings; secret header as above.
3. Test: `./ttn-webhook.sh` using `ttn-uplink.sample.json`

## Gateway relay via generic webhook

If your middleware already posts to the generic endpoint using a **gateway** device token:

```json
{
  "deviceToken": "gateway-access-token",
  "childDeviceId": "child-device-uuid",
  "telemetry": { "temperature": 24.1 }
}
```

## See also

- `../../CONNECTIVITY.md` — full connectivity guide
- `../../../backend/docs/CONNECTIVITY.md` — backend reference
