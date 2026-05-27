"""RPC handlers shared by RPCCommands_* and AllFunctionTest_* examples."""

from __future__ import annotations

import os
import random
import time
from typing import Any, Callable, Dict

from .device_state import DeviceState
from .health import health_attributes
from .telemetry_keys import sample_telemetry


def handle_rpc(
    method: str,
    payload: Dict[str, Any],
    state: DeviceState,
    reply: Callable[[Dict[str, Any]], None],
    send_telemetry: Callable[[str, float], None],
    reboot: Callable[[], None],
) -> None:
    print(f"[RPC] method={method}")
    params = payload.get("params") or {}

    if method == "ping":
        reply(
            {
                "success": True,
                "message": "pong",
                "uptime": int(time.time() * 1000),
            }
        )
        return

    if method == "getStatus":
        h = health_attributes()
        reply(
            {
                "success": True,
                "uptime": h["uptime"],
                "freeHeap": h["freeHeap"],
                "wifiRSSI": h["wifiRSSI"],
                "channel1": 1 if state.channel_state[1] else 0,
                "channel2": 1 if state.channel_state[2] else 0,
                "channel3": 1 if state.channel_state[3] else 0,
                "channel4": 1 if state.channel_state[4] else 0,
                "volume": state.volume,
                "setVoltage1": state.set_voltage1,
                "setVoltage2": state.set_voltage2,
                "setVoltage3": state.set_voltage3,
            }
        )
        return

    if method == "getConfig":
        reply(
            {
                "success": True,
                "sdkVersion": 1.0,
                "platform": "raspberrypi",
                "hostname": os.uname().nodename,
            }
        )
        return

    if method == "getDiagnostics":
        h = health_attributes()
        reply(
            {
                "success": True,
                "uptime": h["uptime"],
                "freeHeap": h["freeHeap"],
                "wifiRSSI": h["wifiRSSI"],
                "platform": "raspberrypi",
            }
        )
        return

    if method == "setValue":
        key = str(params.get("key") or "")
        val = float(params.get("value") or 0)
        if not key:
            reply({"success": False, "message": "key is required"})
            return
        state._client_attr(key, val)
        reply({"success": True, "key": key, "value": val})
        return

    if method == "relay_set":
        ch = int(params.get("channel") or 1)
        on = int(params.get("state") or 0) > 0
        if ch < 1 or ch > 4:
            reply({"success": False, "message": "channel must be 1-4"})
            return
        state.apply_channel(ch, 1.0 if on else 0.0)
        reply({"success": True, "channel": ch, "state": 1 if on else 0})
        return

    if method == "reset":
        state.reset_all()
        reply({"success": True, "message": "Reset complete"})
        return

    if method == "reboot":
        reply({"success": True, "message": "Reboot accepted"})
        time.sleep(1)
        reboot()
        return

    if method == "telemetry_burst":
        count = int(params.get("count") or 5)
        count = max(1, min(20, count))
        for _ in range(count):
            send_telemetry("burstValue", float(random.randint(1, 99)))
            time.sleep(0.3)
        reply({"success": True, "sent": count})
        return

    reply(
        {
            "success": False,
            "message": "Unknown method",
            "method": method,
        }
    )
