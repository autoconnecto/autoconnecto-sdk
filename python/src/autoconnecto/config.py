"""Load settings from environment (see raspberrypi/env.example or python README)."""

from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path

_PACKAGE_DIR = Path(__file__).resolve().parent


@dataclass(frozen=True)
class Settings:
    device_token: str
    mqtt_host: str
    mqtt_port: int
    api_host: str
    ca_file: Path
    insecure_tls: bool
    relay_pins: tuple[int, ...]


def _bundled_ca() -> Path:
    """ISRG Root X1+X2 bundle shipped with the package."""
    return _PACKAGE_DIR / "isrg_roots.pem"


def load_settings() -> Settings:
    token = os.environ.get("AUTOCONNECTO_DEVICE_TOKEN", "").strip()
    if not token:
        raise SystemExit(
            "Set AUTOCONNECTO_DEVICE_TOKEN "
            "(copy raspberrypi/env.example to .env, or export in the shell)"
        )

    ca = os.environ.get("AUTOCONNECTO_CA_FILE", "").strip()
    if ca:
        ca_path = Path(ca)
    else:
        ca_path = _bundled_ca()
    if not ca_path.is_file():
        raise SystemExit(f"CA bundle not found: {ca_path}")

    pins_raw = os.environ.get("AUTOCONNECTO_RELAY_PINS", "2,4,5,18")
    pins: list[int] = []
    for part in pins_raw.split(","):
        part = part.strip()
        if part:
            pins.append(int(part))

    return Settings(
        device_token=token,
        mqtt_host=os.environ.get(
            "AUTOCONNECTO_MQTT_HOST", "mqtt.autoconnecto.in"
        ),
        mqtt_port=int(os.environ.get("AUTOCONNECTO_MQTT_PORT", "8883")),
        api_host=os.environ.get(
            "AUTOCONNECTO_API_HOST", "api.autoconnecto.in"
        ),
        ca_file=ca_path,
        insecure_tls=os.environ.get("AUTOCONNECTO_INSECURE_TLS", "0")
        in ("1", "true", "yes"),
        relay_pins=tuple(pins) if pins else (2, 4, 5, 18),
    )


def load_dotenv_if_present(env_path: Path | None = None) -> None:
    """Load a .env file when python-dotenv is installed (optional)."""
    if env_path is None:
        # Prefer cwd, then common raspberrypi/.env locations
        candidates = [
            Path.cwd() / ".env",
            Path.cwd().parent / ".env",
        ]
        # Walk up a few levels looking for raspberrypi/.env
        here = Path.cwd()
        for _ in range(4):
            candidates.append(here / "raspberrypi" / ".env")
            candidates.append(here / ".env")
            if here.parent == here:
                break
            here = here.parent
        for candidate in candidates:
            if candidate.is_file():
                env_path = candidate
                break
    if env_path is None or not env_path.is_file():
        return
    try:
        from dotenv import load_dotenv

        load_dotenv(env_path)
    except ImportError:
        pass
