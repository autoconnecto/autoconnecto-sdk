#!/usr/bin/env python3
"""SwitchControl_mqtt — Raspberry Pi (pairs with ESP32 example of same name)."""

from __future__ import annotations

import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from common.config import load_dotenv_if_present, load_settings
from common.device_state import DeviceState
from common.health import health_attributes
from common.mqtt_device import AutoconnectoMqttDevice
from common.telemetry_keys import sample_telemetry

load_dotenv_if_present()


def main() -> None:
    settings = load_settings()
    state = DeviceState(relay_pins=settings.relay_pins)
    mqtt = AutoconnectoMqttDevice(settings)

    state.bind_client_publisher(mqtt.send_client_attribute)

    def on_connect() -> None:
        print("[NET] Connected — requesting shared attributes")
        mqtt.request_shared_attributes()

    mqtt.on_attribute_update(state.on_attribute_update)
    mqtt.on_connected(on_connect)
    mqtt.connect()
    time.sleep(2)

    print("[SDK] SwitchControl_mqtt started (raspberrypi)")
    last_telemetry = 0.0
    last_attr = 0.0

    try:
        while True:
            now = time.monotonic()
            if now - last_telemetry >= 10:
                last_telemetry = now
                mqtt.send_telemetry(sample_telemetry())

            if now - last_attr >= 30:
                last_attr = now
                mqtt.send_client_attributes(health_attributes())

            time.sleep(0.05)
    except KeyboardInterrupt:
        print("\n[SDK] Stopped")
    finally:
        mqtt.disconnect()


if __name__ == "__main__":
    main()
