# Autoconnecto Python SDK (Linux SBC)

Installable client for **Raspberry Pi**, Orange Pi, Radxa ROCK, BeagleBone, and Debian/Ubuntu industrial gateways. Same MQTT/HTTPS device contract as the ESP32 Arduino SDK (telemetry, shared attributes, client attributes, RPC), plus **GPIO** via gpiozero (digital out/in and Switch relay channels).

This is a **device client**, not a local platform edge.

## Install (full — MQTT + HTTPS + GPIO)

```bash
cd python
python3 -m venv .venv
source .venv/bin/activate   # Windows: .venv\Scripts\activate
pip install -e ".[gpio,dotenv]"
```

On a headless x86 gateway with no pins, omit gpio: `pip install -e ".[dotenv]"`. GPIO calls then log as simulated.

After PyPI publish:

```bash
pip install "autoconnecto[gpio,dotenv]"
```

## Quick use

```python
import os
os.environ["AUTOCONNECTO_DEVICE_TOKEN"] = "YOUR_TOKEN"

from autoconnecto import AutoconnectoMqttDevice, load_settings, gpio

settings = load_settings()
mqtt = AutoconnectoMqttDevice(settings)
mqtt.connect()
mqtt.send_telemetry({"temperature": 28.5})

# Digital out (BCM numbering on Raspberry Pi)
gpio.write(17, True)
# Digital in
pressed = gpio.read(27)
# Switch channels 1–4 → pins from AUTOCONNECTO_RELAY_PINS (default 2,4,5,18)
gpio.set_relay(1, True, settings.relay_pins)
```

TLS uses the bundled ISRG root CA. Override with `AUTOCONNECTO_CA_FILE` if needed.

## Named boards

| Board | Notes |
|-------|--------|
| **Raspberry Pi 5 / 4 / 3** | Primary — gpiozero + Raspberry Pi OS |
| **Raspberry Pi Zero 2 W** | Same OS/GPIO path; Wi‑Fi |
| **Orange Pi 5** (and similar Allwinner/Rockchip Orange Pi) | Debian images; GPIO when gpiozero/pin factory works on that image |
| **Radxa ROCK 5 / ROCK Pi** | Same class as above |
| **BeagleBone Black / AI** | Debian; confirm pin numbering for your cape |
| **Industrial x86 / ARM Debian gateways** | MQTT/HTTPS full; GPIO only if the vendor exposes Linux GPIO |

## Examples

Runnable scripts mirroring ESP32 sketch names: [`../raspberrypi/examples/`](../raspberrypi/examples/). Prefer `pip install -e ".[gpio,dotenv]"` before running SwitchControl / AllFunctionTest on a Pi.

## Version

Tracks the Autoconnecto SDK release line (`pyproject.toml` / `library.properties`).
