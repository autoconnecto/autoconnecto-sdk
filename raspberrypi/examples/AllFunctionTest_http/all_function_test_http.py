#!/usr/bin/env python3
"""AllFunctionTest_http — Raspberry Pi (pairs with ESP32 example of same name)."""

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
from common.http_device import AutoconnectoHttpDevice
from common.http_shared_poll import SharedAttributePoller
from common.telemetry_keys import sample_telemetry

load_dotenv_if_present()

POLL_SHARED_MS = 0.75


def main() -> None:
    settings = load_settings()
    state = DeviceState(relay_pins=settings.relay_pins)
    http = AutoconnectoHttpDevice(settings)
    state.bind_client_publisher(http.post_client_attribute)
    poller = SharedAttributePoller(http, state.on_attribute_update)

    code, _ = http.get_healthz()
    print(f"[HEALTHZ] {code}")
    print("[HTTP] AllFunctionTest_http started (raspberrypi)")

    last_telemetry = 0.0
    last_attr = 0.0
    last_poll = 0.0

    try:
        while True:
            now = time.monotonic()
            if now - last_poll >= POLL_SHARED_MS:
                last_poll = now
                poller.poll()

            if now - last_telemetry >= 10:
                last_telemetry = now
                http.post_telemetry(sample_telemetry())

            if now - last_attr >= 30:
                last_attr = now
                http.post_client_attributes(health_attributes())

            time.sleep(0.05)
    except KeyboardInterrupt:
        print("\n[HTTP] Stopped")


if __name__ == "__main__":
    main()
