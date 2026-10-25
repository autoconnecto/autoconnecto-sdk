"""GPIO helpers for Linux SBCs (gpiozero).

Digital outs drive dashboard Switch / relay channels. Digital ins can be
published as telemetry or client attributes. When gpiozero or the pin
backend is missing (x86 VM, non-Pi board without a factory), writes are
simulated in the log so MQTT examples still run.
"""

from __future__ import annotations

from typing import Dict, Optional

_outputs: Dict[int, object] = {}
_inputs: Dict[int, object] = {}
_simulated_out: Dict[int, bool] = {}
_simulated_in: Dict[int, bool] = {}


def _backend_available() -> bool:
    try:
        import gpiozero  # noqa: F401

        return True
    except ImportError:
        return False


def write(pin: int, on: bool) -> None:
    """Drive a BCM (or board-native) digital output high/low."""
    try:
        from gpiozero import OutputDevice

        if pin not in _outputs:
            _outputs[pin] = OutputDevice(
                pin, active_high=True, initial_value=False
            )
        dev = _outputs[pin]
        if on:
            dev.on()
        else:
            dev.off()
        print(f"[GPIO] out pin {pin} = {'ON' if on else 'OFF'}")
    except Exception as exc:
        _simulated_out[pin] = on
        print(
            f"[GPIO] out pin {pin} = {'ON' if on else 'OFF'} "
            f"(simulated; {exc.__class__.__name__})"
        )


def read(pin: int, *, pull_up: bool = True) -> bool:
    """Read a digital input. Returns False when GPIO is unavailable."""
    try:
        from gpiozero import Button, DigitalInputDevice

        if pin not in _inputs:
            # Button gives stable pull; DigitalInputDevice if no bounce needed
            try:
                _inputs[pin] = Button(pin, pull_up=pull_up)
            except Exception:
                _inputs[pin] = DigitalInputDevice(pin, pull_up=pull_up)
        dev = _inputs[pin]
        # Button.is_pressed / DigitalInputDevice.is_active
        if hasattr(dev, "is_pressed"):
            return bool(dev.is_pressed)
        return bool(getattr(dev, "is_active", False))
    except Exception as exc:
        val = _simulated_in.get(pin, False)
        print(
            f"[GPIO] in pin {pin} = {val} "
            f"(simulated; {exc.__class__.__name__})"
        )
        return val


def set_simulated_input(pin: int, on: bool) -> None:
    """For desk tests without hardware — next read() returns this value."""
    _simulated_in[pin] = on


def set_relay(channel: int, on: bool, pins: tuple[int, ...]) -> None:
    """Map Switch channel 1..N to pins (same as ESP32 SwitchControl examples)."""
    if channel < 1 or channel > len(pins):
        return
    pin = pins[channel - 1]
    write(pin, on)
    print(f"[GPIO] channel{channel} → pin {pin}")


def gpio_ready() -> bool:
    """True when gpiozero imported; pin access still depends on the board."""
    return _backend_available()
