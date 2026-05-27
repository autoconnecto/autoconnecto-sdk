#!/usr/bin/env python3
"""BasicTelemetry_mqtt — Raspberry Pi (pairs with ESP32 example of same name)."""

from __future__ import annotations

import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from common.config import load_dotenv_if_present, load_settings
from common.health import health_attributes
from common.mqtt_device import AutoconnectoMqttDevice
from common.telemetry_keys import sample_telemetry

load_dotenv_if_present()


def main() -> None:
    settings = load_settings()
    mqtt = AutoconnectoMqttDevice(settings)
    mqtt.connect()
    time.sleep(2)

    print("[SDK] BasicTelemetry_mqtt started (raspberrypi)")
    last_telemetry = 0.0
    last_attr = 0.0

    try:
        while True:
            now = time.monotonic()
            if now - last_telemetry >= 10:
                last_telemetry = now
                mqtt.send_telemetry(sample_telemetry())
                print("[TEL] Sent telemetry")

            if now - last_attr >= 30:
                last_attr = now
                mqtt.send_client_attributes(health_attributes())
                print("[ATTR] Sent device health")

            time.sleep(0.05)
    except KeyboardInterrupt:
        print("\n[SDK] Stopped")
    finally:
        mqtt.disconnect()


if __name__ == "__main__":
    main()
