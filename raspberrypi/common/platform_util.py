"""Platform helpers for examples."""

from __future__ import annotations

import os
import subprocess


def reboot_device() -> None:
    print("[RPC] Rebooting...")
    try:
        subprocess.run(["sudo", "reboot"], check=False)
    except OSError:
        pass
    try:
        os.execv("/sbin/reboot", ["reboot"])
    except OSError:
        print("[RPC] reboot failed — run: sudo reboot")
