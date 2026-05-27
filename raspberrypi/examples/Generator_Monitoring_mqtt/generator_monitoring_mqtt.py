#!/usr/bin/env python3
"""Generator_Monitoring_mqtt — Raspberry Pi mirror of ESP32 generator demo."""

from __future__ import annotations

import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from common.config import load_dotenv_if_present, load_settings
from common.mqtt_device import AutoconnectoMqttDevice

load_dotenv_if_present()


def main() -> None:
    settings = load_settings()
    mqtt = AutoconnectoMqttDevice(settings)
    mqtt.connect()
    time.sleep(2)

    print("[SDK] Generator_Monitoring_mqtt started (raspberrypi)")

    phase = "stopped"
    phase_started = time.monotonic()
    run_hours = 1200.0
    fuel_pct = 72.0
    tick = 0

    last_tel = 0.0
    last_attrs = 0.0

    def send_rated_attrs() -> None:
        mqtt.send_client_attributes(
            {
                "gen_rated_kva": 200,
                "gen_nominal_v": 415,
                "gen_nominal_hz": 50,
            }
        )

    def advance_phase() -> None:
        nonlocal phase, phase_started, tick
        elapsed = time.monotonic() - phase_started

        if phase == "stopped" and elapsed > 8:
            phase = "starting"
            phase_started = time.monotonic()
            return
        if phase == "starting" and elapsed > 4:
            phase = "running"
            phase_started = time.monotonic()
            return
        if phase == "running" and elapsed > 45:
            phase = "stopped"
            phase_started = time.monotonic()
            return
        if phase == "running" and elapsed > 20 and (tick % 6) == 5:
            phase = "fault"
            phase_started = time.monotonic()
            return
        if phase == "fault" and elapsed > 12:
            phase = "stopped"
            phase_started = time.monotonic()

    def send_telemetry() -> None:
        nonlocal run_hours, fuel_pct, tick
        advance_phase()
        tick += 1

        running = phase == "running"
        starting = phase == "starting"
        fault = phase == "fault"

        load_pct = 55.0 + (tick % 20) if running else (8.0 if starting else 0.0)
        kw = (load_pct / 100.0) * 180.0 if running else 0.0
        kva = kw / 0.92 if running else 0.0
        base_v = 238.0 if running else (220.0 if starting else 0.0)

        if running:
            run_hours += 10.0 / 3600.0
            fuel_pct = max(5.0, fuel_pct - 0.02)

        tel = {
            "gen_run_state": phase,
            "gen_mode": "auto",
            "gen_on_load": running,
            "gen_kw": kw,
            "gen_kva": kva,
            "gen_pf": 0.91 if running else 0.0,
            "gen_hz": 50.02 if running else (49.5 if starting else 0.0),
            "gen_load_pct": load_pct,
            "gen_v_l1": base_v + (0.4 if running else 0.0),
            "gen_v_l2": base_v - 0.2,
            "gen_v_l3": base_v + 0.1,
            "gen_i_l1": 210.0 + (tick % 5) if running else 0.0,
            "gen_i_l2": 215.0 if running else 0.0,
            "gen_i_l3": 208.0 if running else 0.0,
            "gen_rpm": 1500.0 if running else (900.0 if starting else 0.0),
            "gen_oil_pressure": 4.1 + (tick % 3) * 0.05 if running else 0.0,
            "gen_coolant_temp": 82.0 + (tick % 4) if running else (45.0 if starting else 28.0),
            "gen_fuel_level_pct": fuel_pct,
            "gen_battery_v": 27.2,
            "gen_run_hours": run_hours,
            "gen_alarm_count": 1 if fault else 0,
            "gen_alarm_text": "Simulated over-temperature" if fault else "",
        }

        mqtt.send_telemetry(tel)
        print("[TEL] Generator telemetry sent")

    try:
        while True:
            now = time.monotonic()

            if now - last_tel >= 10:
                last_tel = now
                send_telemetry()

            if now - last_attrs >= 60:
                last_attrs = now
                send_rated_attrs()
                print("[ATTR] Rated generator attributes sent")

            time.sleep(0.05)
    except KeyboardInterrupt:
        print("\n[SDK] Stopped")
    finally:
        mqtt.disconnect()


if __name__ == "__main__":
    main()

