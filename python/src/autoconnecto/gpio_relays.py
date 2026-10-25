"""Back-compat — prefer ``autoconnecto.gpio``."""

from .gpio import set_relay

__all__ = ["set_relay"]
