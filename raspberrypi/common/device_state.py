"""Shared attribute handling — mirrors SwitchControl / AllFunctionTest ESP32 logic."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Callable, Optional

from .gpio_relays import set_relay


@dataclass
class DeviceState:
    channel_state: list[bool] = field(
        default_factory=lambda: [False] * 5
    )
    volume: float = 0.0
    limit_voltage1: float = 0.0
    limit_voltage2: float = 0.0
    limit_voltage3: float = 0.0
    set_voltage1: float = 245.0
    set_voltage2: float = 245.0
    set_voltage3: float = 245.0
    relay_pins: tuple[int, ...] = (2, 4, 5, 18)
    _publish_client: Optional[Callable[[str, float], None]] = None

    def bind_client_publisher(
        self, publish: Callable[[str, float], None]
    ) -> None:
        self._publish_client = publish

    def apply_channel(self, ch: int, raw: float) -> None:
        if ch < 1 or ch > 4:
            return
        state = raw > 0.5
        self.channel_state[ch] = state
        set_relay(ch, state, self.relay_pins)
        self._client_attr(f"channel{ch}", 1.0 if state else 0.0)
        print(f"[ATTR] channel{ch} = {'ON' if state else 'OFF'}")

    def on_attribute_update(self, key: str, value: float) -> None:
        print(f"[ATTR] {key} = {value:.2f}")

        if key == "channel1":
            self.apply_channel(1, value)
            return
        if key == "channel2":
            self.apply_channel(2, value)
            return
        if key == "channel3":
            self.apply_channel(3, value)
            return
        if key == "channel4":
            self.apply_channel(4, value)
            return

        if key == "volume":
            self.volume = value
            self._client_attr("volume", self.volume)
            return

        if key == "limitVoltage1":
            self.limit_voltage1 = value
            self.set_voltage1 = value
            self._client_attr("setVoltage1", self.set_voltage1)
            return
        if key == "limitVoltage2":
            self.limit_voltage2 = value
            self.set_voltage2 = value
            self._client_attr("setVoltage2", self.set_voltage2)
            return
        if key == "limitVoltage3":
            self.limit_voltage3 = value
            self.set_voltage3 = value
            self._client_attr("setVoltage3", self.set_voltage3)
            return

        self._client_attr(key, value)

    def _client_attr(self, key: str, value: float) -> None:
        if self._publish_client:
            self._publish_client(key, value)

    def reset_all(self) -> None:
        for i in range(1, 5):
            self.apply_channel(i, 0.0)
        self.volume = 0.0
        self._client_attr("volume", 0.0)
        self.limit_voltage1 = 0.0
        self.limit_voltage2 = 0.0
        self.limit_voltage3 = 0.0
        self.set_voltage1 = 245.0
        self.set_voltage2 = 245.0
        self.set_voltage3 = 245.0
        self._client_attr("setVoltage1", self.set_voltage1)
        self._client_attr("setVoltage2", self.set_voltage2)
        self._client_attr("setVoltage3", self.set_voltage3)
