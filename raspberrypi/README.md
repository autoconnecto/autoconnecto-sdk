# Autoconnecto — Raspberry Pi / Linux SBC examples

Python examples that mirror the ESP32 sketches under `examples/*.ino`. They use the **same telemetry keys, attributes, and RPC methods** so one dashboard works for both platforms.

**Installable SDK:** [`../python/`](../python/) — full install `pip install -e "../python[gpio,dotenv]"` (package **`autoconnecto`**: MQTT, HTTPS, GPIO). Same client the examples import.

**Matching dashboards:** in the Autoconnecto app, open **Solutions** and search `SDK`. Each example has a template titled **`SDK · <Arduino example folder>`** (same keys as these Python scripts).

## Requirements

- Raspberry Pi OS (or any Linux) with Python **3.9+**
- Network access to `mqtt.autoconnecto.in` and/or `api.autoconnecto.in`
- Device token from the Autoconnecto platform

## Quick start

```bash
cd sdk/python
python3 -m venv .venv
source .venv/bin/activate   # Windows: .venv\Scripts\activate
pip install -e ".[gpio,dotenv]"

cd ../raspberrypi
cp env.example .env
# Edit .env — set AUTOCONNECTO_DEVICE_TOKEN

python3 examples/BasicTelemetry_mqtt/basic_telemetry_mqtt.py
```

Examples also add `python/src` to `sys.path` so a git checkout works without install; prefer `pip install -e ".[gpio,dotenv]"` for production and systemd.

Named boards: Raspberry Pi 5 / 4 / 3 / Zero 2 W, Orange Pi 5, Radxa ROCK 5, BeagleBone, industrial Debian gateways.

## Run on boot (systemd)

1. Install the package into a venv at a stable path, e.g. `/opt/autoconnecto-sdk/python/.venv`.
2. Put `.env` under `/opt/autoconnecto-sdk/raspberrypi/.env`.
3. Point the unit’s `ExecStart` at that venv’s Python and the example script (see `systemd/autoconnecto-basic-telemetry.service`).

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

See `env.example`. TLS uses the CA bundled in the `autoconnecto` package (same ISRG X1+X2 as `src/AutoconnectoIsrgRoots.h`). Override with `AUTOCONNECTO_CA_FILE` if needed. A copy remains under `ca/` for reference.

## GPIO (optional)

Relays default to BCM pins `2,4,5,18` (same as ESP32 examples). Override with `AUTOCONNECTO_RELAY_PINS=2,4,5,18`.

Install `gpiozero` (`pip install "autoconnecto[gpio]"`) on the Pi for real outputs; without it, examples **simulate** relay state in the log.

## Layout

```text
python/                 # installable package (pip install -e ./python)
  src/autoconnecto/
raspberrypi/
  common/               # thin shims → autoconnecto (compat)
  ca/                   # reference TLS roots
  examples/             # one folder per ESP32 example name
  requirements.txt      # points at ../python
```
