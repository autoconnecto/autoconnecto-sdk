#!/usr/bin/env python3
"""Post sample telemetry to the Autoconnecto generic integration webhook."""

import json
import os
import sys
import time
import urllib.error
import urllib.request

API_BASE = os.environ.get("API_BASE", "https://api.autoconnecto.in").rstrip("/")
TENANT_ID = os.environ.get("TENANT_ID", "")
WEBHOOK_SECRET = os.environ.get("WEBHOOK_SECRET", "")
DEVICE_TOKEN = os.environ.get("DEVICE_TOKEN", "")
DEVICE_ID = os.environ.get("DEVICE_ID", "")


def main() -> int:
    if not TENANT_ID or not WEBHOOK_SECRET:
        print("Set TENANT_ID and WEBHOOK_SECRET", file=sys.stderr)
        return 1

    url = f"{API_BASE}/api/v1/integrations/generic/telemetry?tenantId={TENANT_ID}"

    if DEVICE_TOKEN:
        payload = {
            "deviceToken": DEVICE_TOKEN,
            "temperature": 22.5,
            "humidity": 58,
            "ts": int(time.time() * 1000),
        }
    elif DEVICE_ID:
        payload = {
            "externalDeviceId": DEVICE_ID,
            "telemetry": {"temperature": 22.5, "humidity": 58},
        }
    else:
        print("Set DEVICE_TOKEN or DEVICE_ID", file=sys.stderr)
        return 1

    data = json.dumps(payload).encode("utf-8")
    req = urllib.request.Request(
        url,
        data=data,
        method="POST",
        headers={
            "Content-Type": "application/json",
            "X-Webhook-Secret": WEBHOOK_SECRET,
        },
    )

    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            print(resp.status, resp.read().decode("utf-8", errors="replace"))
    except urllib.error.HTTPError as e:
        print(e.code, e.read().decode("utf-8", errors="replace"), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
