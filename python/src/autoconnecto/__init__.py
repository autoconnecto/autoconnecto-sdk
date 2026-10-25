"""Autoconnecto Python SDK for Linux SBCs (MQTT + HTTPS + GPIO)."""

from . import gpio
from .config import Settings, load_dotenv_if_present, load_settings
from .http_device import AutoconnectoHttpDevice
from .http_shared_poll import SharedAttributePoller
from .mqtt_device import AutoconnectoMqttDevice

__version__ = "1.7.79"

__all__ = [
    "Settings",
    "load_settings",
    "load_dotenv_if_present",
    "AutoconnectoMqttDevice",
    "AutoconnectoHttpDevice",
    "SharedAttributePoller",
    "gpio",
    "__version__",
]
