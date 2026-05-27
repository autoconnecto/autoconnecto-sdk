#!/usr/bin/env python3
"""BasicTelemetry_http — Raspberry Pi (pairs with ESP32 example of same name)."""

from __future__ import annotations

import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from common.config import load_dotenv_if_present, load_settings
from common.health import health_attributes
from common.http_device import AutoconnectoHttpDevice
from common.telemetry_keys import sample_telemetry

load_dotenv_if_present()


def main() -> None:
    settings = load_settings()
    http = AutoconnectoHttpDevice(settings)
    code, _ = http.get_healthz()
    print(f"[HEALTHZ] {code}")
    print("[HTTP] BasicTelemetry_http started (raspberrypi)")

    last_telemetry = 0.0
    last_attr = 0.0

    try:
        while True:
            now = time.monotonic()
            if now - last_telemetry >= 10:
                last_telemetry = now
                code = http.post_telemetry(sample_telemetry())
                print(f"[TELEMETRY] {code}")

            if now - last_attr >= 30:
                last_attr = now
                code = http.post_client_attributes(health_attributes())
                print(f"[CLIENT ATTR] {code}")

            time.sleep(0.05)
    except KeyboardInterrupt:
        print("\n[HTTP] Stopped")


if __name__ == "__main__":
    main()
