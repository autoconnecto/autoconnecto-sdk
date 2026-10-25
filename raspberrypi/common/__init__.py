"""Compatibility shim — prefer `import autoconnecto` after `pip install autoconnecto`."""

from autoconnecto import (  # noqa: F401
    AutoconnectoHttpDevice,
    AutoconnectoMqttDevice,
    Settings,
    SharedAttributePoller,
    load_dotenv_if_present,
    load_settings,
)
