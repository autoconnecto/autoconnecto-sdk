"""Poll shared attributes over HTTP (SwitchControl_http pattern)."""

from __future__ import annotations

from typing import Callable, Dict, Tuple

from .http_device import AutoconnectoHttpDevice


def _to_float(val) -> float:
    if val is None:
        return 0.0
    if isinstance(val, bool):
        return 1.0 if val else 0.0
    if isinstance(val, (int, float)):
        return float(val)
    try:
        return float(val)
    except (TypeError, ValueError):
        return 0.0


class SharedAttributePoller:
    def __init__(
        self,
        http: AutoconnectoHttpDevice,
        on_update: Callable[[str, float], None],
    ) -> None:
        self._http = http
        self._on_update = on_update
        self._tracked: Dict[str, float] = {}

    def poll(self) -> None:
        data = self._http.get_shared_attributes_flat()
        if data is None:
            return

        items = data if isinstance(data, list) else []

        for item in items:
            if not isinstance(item, dict):
                continue
            if str(item.get("scope", "")).upper() != "SHARED":
                continue
            key = str(item.get("key") or "")
            if not key:
                continue
            fval = _to_float(item.get("value"))
            prev = self._tracked.get(key)
            if prev is None or abs(prev - fval) > 1e-4:
                self._tracked[key] = fval
                self._on_update(key, fval)
