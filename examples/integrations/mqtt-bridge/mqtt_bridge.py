#!/usr/bin/env python3
"""Bridge a source MQTT topic to Autoconnecto device telemetry."""

from __future__ import annotations

import json
import ssl
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict

import paho.mqtt.client as mqtt


@dataclass
class Config:
    src_host: str
    src_port: int
    src_topic: str
    device_token: str
    ac_host: str
    ac_port: int
    ca_file: Path
    insecure_tls: bool
    mode: str
    wrap_key: str


def load_config(path: Path) -> Config:
    raw = json.loads(path.read_text(encoding="utf-8"))
    src = raw.get("source") or {}
    ac = raw.get("autoconnecto") or {}
    return Config(
        src_host=str(src.get("host", "127.0.0.1")),
        src_port=int(src.get("port", 1883)),
        src_topic=str(src.get("topic", "sensors/+/telemetry")),
        device_token=str(ac.get("deviceToken", "")),
        ac_host=str(ac.get("mqttHost", "mqtt.autoconnecto.in")),
        ac_port=int(ac.get("mqttPort", 8883)),
        ca_file=Path(str(ac.get("caFile", ""))).resolve(),
        insecure_tls=bool(ac.get("insecureTls", False)),
        mode=str(raw.get("mode", "forward_json")),
        wrap_key=str(raw.get("wrapKey", "value")),
    )


def make_ac_mqtt(cfg: Config) -> mqtt.Client:
    c = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=cfg.device_token)
    c.username_pw_set(cfg.device_token, None)
    if cfg.insecure_tls:
        c.tls_set(cert_reqs=ssl.CERT_NONE)
        c.tls_insecure_set(True)
    else:
        if not cfg.ca_file.is_file():
            raise SystemExit(f"CA bundle not found: {cfg.ca_file}")
        c.tls_set(ca_certs=str(cfg.ca_file))
    return c


def make_src_mqtt(cfg: Config, on_msg) -> mqtt.Client:
    c = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="autoconnecto-bridge")
    c.on_message = on_msg

    def on_connect(client, userdata, flags, reason_code, properties=None):
        if reason_code != 0:
            print(f"[SRC] connect failed: {reason_code}")
            return
        print("[SRC] connected")
        client.subscribe(cfg.src_topic, qos=0)

    c.on_connect = on_connect
    return c


def main() -> int:
    if len(sys.argv) != 2:
        print("Usage: python mqtt_bridge.py config.json", file=sys.stderr)
        return 2

    cfg = load_config(Path(sys.argv[1]).resolve())
    if not cfg.device_token:
        raise SystemExit("config.autoconnecto.deviceToken is required")

    ac = make_ac_mqtt(cfg)
    ac.connect(cfg.ac_host, cfg.ac_port, keepalive=60)
    ac.loop_start()
    ac_topic = f"devices/{cfg.device_token}/telemetry"
    print(f"[AC] connected -> publish {ac_topic}")

    def on_msg(client, userdata, msg: mqtt.MQTTMessage) -> None:
        try:
            raw = msg.payload.decode("utf-8", errors="strict")
        except UnicodeDecodeError:
            return

        out: Dict[str, Any]
        if cfg.mode == "wrap_scalar":
            out = {cfg.wrap_key: raw}
        else:
            try:
                parsed = json.loads(raw)
            except json.JSONDecodeError:
                return
            if isinstance(parsed, dict):
                out = parsed
            else:
                out = {cfg.wrap_key: parsed}

        ac.publish(ac_topic, json.dumps(out).encode("utf-8"), qos=1)

    src = make_src_mqtt(cfg, on_msg)
    src.connect(cfg.src_host, cfg.src_port, keepalive=60)
    src.loop_forever()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

