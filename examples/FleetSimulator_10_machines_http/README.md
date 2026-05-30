# FleetSimulator_10_machines_http

One ESP32 simulates **10 devices** by rotating HTTPS telemetry + client attributes across 10 device tokens.

## Get device tokens

Tokens are **not** the same as device IDs. Copy each token from the app:

**Devices → open device → copy Device Token**

Or SQL (local/prod Postgres):

```sql
SELECT device_id, device_name, device_token
FROM devices
WHERE device_id IN (
  '88d6da44-3a99-4373-94c7-9d1d1aa40be7',
  'ff13ef77-56f0-4805-b865-2495ffd49734',
  '88b88a74-4af8-4424-9461-c97c2ce7088a',
  'e4f9e53f-f312-46e0-91bd-63db4644c05a',
  '2e4a59b6-535f-4cfe-ac73-5706a66b3038',
  'afb363fb-0b29-421e-b291-8c06aca19ece',
  '72c6a849-2ce5-4760-94a5-e1bfb673d74a',
  '950848a4-d873-4779-b433-b6d99605fc53',
  '5ea53525-9868-4be3-bbd8-b9f2f1868395',
  '840415bf-a261-45b1-a76a-4fd29d896266'
)
ORDER BY device_name;
```

Paste into `REPLACE_WITH_TOKEN_1` … `REPLACE_WITH_TOKEN_10` in the `.ino` file.

## Arduino IDE

1. Install **AutoconnectoSDK** and **ArduinoJson**.
2. Open `FleetSimulator_10_machines_http.ino`.
3. Set Wi‑Fi and tokens.
4. Upload to ESP32.

## What it sends (per device, every ~12 s)

| Key | Use |
|-----|-----|
| `machine_current_a` | Machine Fleet widget |
| `machine_running` | Running/stopped hint |
| `temperature`, `humidity`, `current`, `power`, `voltage1..3` | Standard charts / pipelines |
| Client attrs: `freeHeap`, `wifiRSSI`, `uptime`, `fleet_label` | Device detail / attributes |

## Local dev

```cpp
static const char* API_HOST = "app.local.autoconnecto";
#define USE_INSECURE_TLS_DEBUG 1
```

## Production

```cpp
static const char* API_HOST = "api.autoconnecto.in";
#define USE_INSECURE_TLS_DEBUG 0
```

## MQTT alternative

For full RPC + shared-attribute testing per device, flash **one device per board** using `AllFunctionTest_mqtt.ino` with each token. This HTTP sketch is for fleet volume testing from a single board.
