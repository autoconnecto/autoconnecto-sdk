"""Optional relay outputs (gpiozero). Simulates when GPIO is unavailable."""

from __future__ import annotations

from typing import Dict, Optional

_outputs: Dict[int, object] = {}
_simulated: Dict[int, bool] = {}


def set_relay(channel: int, on: bool, pins: tuple[int, ...]) -> None:
    if channel < 1 or channel > len(pins):
        return
    pin = pins[channel - 1]

    try:
        from gpiozero import OutputDevice

        if pin not in _outputs:
            _outputs[pin] = OutputDevice(pin, active_high=True, initial_value=False)
        dev = _outputs[pin]
        dev.on() if on else dev.off()
        print(f"[GPIO] pin {pin} channel{channel} = {'ON' if on else 'OFF'}")
    except Exception as exc:
        _simulated[channel] = on
        print(
            f"[GPIO] channel{channel} = {'ON' if on else 'OFF'} "
            f"(simulated; {exc.__class__.__name__})"
        )
