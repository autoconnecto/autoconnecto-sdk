"""Standard telemetry payload — same keys as ESP32 examples."""

from __future__ import annotations

import random


def sample_telemetry() -> dict:
    return {
        "temperature": round(random.uniform(20, 35), 2),
        "humidity": round(random.uniform(40, 80), 2),
        "current": round(random.uniform(1, 10), 2),
        "power": round(random.uniform(100, 500), 2),
        "voltage1": round(random.uniform(220, 280), 2),
        "voltage2": round(random.uniform(220, 280), 2),
        "voltage3": round(random.uniform(220, 280), 2),
    }
