"""Load settings from environment (see raspberrypi/env.example)."""

from __future__ import annotations

import os
from dataclasses import dataclass
from pathlib import Path

RASPBERRYPI_ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class Settings:
    device_token: str
    mqtt_host: str
    mqtt_port: int
    api_host: str
    ca_file: Path
    insecure_tls: bool
    relay_pins: tuple[int, ...]


def load_settings() -> Settings:
    token = os.environ.get("AUTOCONNECTO_DEVICE_TOKEN", "").strip()
    if not token:
        raise SystemExit(
            "Set AUTOCONNECTO_DEVICE_TOKEN (copy raspberrypi/env.example to .env)"
        )

    ca = os.environ.get("AUTOCONNECTO_CA_FILE", "").strip()
    ca_path = Path(ca) if ca else RASPBERRYPI_ROOT / "ca" / "isrg_roots.pem"
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


def load_dotenv_if_present() -> None:
    """Load raspberrypi/.env when python-dotenv is installed (optional)."""
    env_path = RASPBERRYPI_ROOT / ".env"
    if not env_path.is_file():
        return
    try:
        from dotenv import load_dotenv

        load_dotenv(env_path)
    except ImportError:
        pass
