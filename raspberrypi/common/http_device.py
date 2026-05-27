"""HTTPS device-token API — matches BasicTelemetry_http / SwitchControl_http."""

from __future__ import annotations

import json
import time
from typing import Any, Dict, Optional, Tuple, Union

import requests

from .config import Settings


class AutoconnectoHttpDevice:
    def __init__(self, settings: Settings) -> None:
        self.settings = settings
        self._token = settings.device_token
        self._session = requests.Session()
        self._session.headers.update({"Content-Type": "application/json"})
        if settings.insecure_tls:
            self._session.verify = False
        else:
            self._session.verify = str(settings.ca_file)

    def _url(self, path: str) -> str:
        return f"https://{self.settings.api_host}{path}"

    def _api(self, suffix: str) -> str:
        return f"/api/v1/{self._token}{suffix}"

    def get_healthz(self) -> Tuple[int, str]:
        r = self._session.get(self._url("/healthz"), timeout=25)
        return r.status_code, r.text

    def post_telemetry(self, body: Dict[str, Any]) -> int:
        r = self._session.post(
            self._url(self._api("/telemetry")),
            data=json.dumps(body),
            timeout=25,
        )
        return r.status_code

    def post_client_attributes(self, body: Dict[str, Any]) -> int:
        r = self._session.post(
            self._url(self._api("/attributes")),
            data=json.dumps(body),
            timeout=25,
        )
        return r.status_code

    def post_client_attribute(self, key: str, value: float) -> int:
        return self.post_client_attributes({key: value})

    def get_shared_attributes_flat(self) -> Optional[Any]:
        r = self._session.get(
            self._url(self._api("/attributes/flat")),
            params={"scope": "SHARED"},
            timeout=25,
        )
        if r.status_code != 200:
            print(f"[HTTP] shared poll failed {r.status_code}")
            return None
        try:
            return r.json()
        except json.JSONDecodeError:
            return None
