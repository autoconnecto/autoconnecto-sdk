"""Device health client attributes (Pi equivalents of ESP32 examples)."""

from __future__ import annotations

import os
import time

_START = time.time()


def health_attributes() -> dict:
    free_kb = 0
    try:
        with open("/proc/meminfo", encoding="utf-8") as f:
            for line in f:
                if line.startswith("MemAvailable:"):
                    free_kb = int(line.split()[1])
                    break
    except OSError:
        pass

    return {
        "freeHeap": free_kb * 1024,
        "wifiRSSI": 0,
        "uptime": int(time.time() - _START),
        "sdkVersion": 1.0,
        "platform": "raspberrypi",
    }
