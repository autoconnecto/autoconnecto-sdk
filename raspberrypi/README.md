# Autoconnecto — Raspberry Pi examples

Python examples that mirror the ESP32 sketches under `examples/*.ino`. They use the **same telemetry keys, attributes, and RPC methods** so one dashboard works for both platforms.

This folder is **not** part of the Arduino Library Manager install (ESP32 only). It ships in the same Git repo as a sibling of `src/` and `examples/`.

## Requirements

- Raspberry Pi OS (or any Linux) with Python **3.9+**
- Network access to `mqtt.autoconnecto.in` and/or `api.autoconnecto.in`
- Device token from the Autoconnecto platform

## Quick start

```bash
cd raspberrypi
python3 -m venv .venv
source .venv/bin/activate   # Windows: .venv\Scripts\activate
pip install -r requirements.txt

cp env.example .env
# Edit .env — set AUTOCONNECTO_DEVICE_TOKEN

python3 examples/BasicTelemetry_mqtt/basic_telemetry_mqtt.py
```

Optional: `pip install python-dotenv` to load `.env` automatically.

## Run on boot (systemd)

1. Copy the repo (or just `raspberrypi/`) to a stable path, e.g. `/opt/autoconnecto-sdk/`.
2. Create venv + install deps there, and create `/opt/autoconnecto-sdk/raspberrypi/.env`.
3. Install the unit:

```bash
sudo cp systemd/autoconnecto-basic-telemetry.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now autoconnecto-basic-telemetry.service
sudo journalctl -u autoconnecto-basic-telemetry.service -f
```

## Examples (mirror ESP32)

| Folder | Script | Transport |
|--------|--------|-----------|
| `BasicTelemetry_mqtt` | `basic_telemetry_mqtt.py` | MQTTS |
| `BasicTelemetry_http` | `basic_telemetry_http.py` | HTTPS |
| `SwitchControl_mqtt` | `switch_control_mqtt.py` | MQTTS + shared attrs |
| `SwitchControl_http` | `switch_control_http.py` | HTTPS poll shared attrs |
| `RPCCommands_mqtt` | `rpc_commands_mqtt.py` | MQTTS + RPC |
| `RPCCommands_http` | `rpc_commands_http.py` | HTTPS (no RPC) |
| `AllFunctionTest_mqtt` | `all_function_test_mqtt.py` | Full MQTT demo |
| `AllFunctionTest_http` | `all_function_test_http.py` | Full HTTP demo |
| `Generator_Monitoring_mqtt` | `generator_monitoring_mqtt.py` | MQTTS (Generator Monitoring widget) |

Server/integration samples remain under `examples/integrations/` (not duplicated here).

## Configuration

See `env.example`. TLS uses `ca/isrg_roots.pem` (same ISRG X1+X2 bundle as `src/AutoconnectoIsrgRoots.h`).

## GPIO (optional)

Relays default to BCM pins `2,4,5,18` (same as ESP32 examples). Override with `AUTOCONNECTO_RELAY_PINS=2,4,5,18`.

Install `gpiozero` on the Pi for real outputs; without it, examples **simulate** relay state in the log.

## Layout

```text
raspberrypi/
  common/          # shared helpers (not a separate PyPI package)
  ca/              # TLS roots
  examples/        # one folder per ESP32 example name
  requirements.txt
```
