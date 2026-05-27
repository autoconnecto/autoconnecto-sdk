#!/usr/bin/env python3
"""OPC-UA gateway -> Autoconnecto telemetry over MQTTS."""

from __future__ import annotations

import asyncio
import json
import ssl
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

import paho.mqtt.client as mqtt
from asyncua import Client as UaClient


def _coerce_json(v: Any) -> Any:
    if v is None:
        return None
    if isinstance(v, (bool, int, float, str)):
        return v
    # asyncua may return enums/variants; stringify as a safe fallback
    return str(v)


@dataclass(frozen=True)
class MappingItem:
    key: str
    node_id: str


@dataclass
class Config:
    opcua_endpoint: str
    opcua_username: str
    opcua_password: str
    device_token: str
    mqtt_host: str
    mqtt_port: int
    ca_file: Path
    insecure_tls: bool
    poll_interval_ms: int
    mapping: List[MappingItem]


def load_config(path: Path) -> Config:
    raw = json.loads(path.read_text(encoding="utf-8"))
    opc = raw.get("opcua") or {}
    ac = raw.get("autoconnecto") or {}
    mapping_raw = raw.get("mapping") or []
    mapping = [
        MappingItem(
            key=str(m.get("key", "")),
            node_id=str(m.get("nodeId", "")),
        )
        for m in mapping_raw
        if isinstance(m, dict) and m.get("key") and m.get("nodeId")
    ]
    if not mapping:
        raise SystemExit("config.mapping must contain at least one {key,nodeId}")

    ca = Path(str(ac.get("caFile", ""))).resolve()
    return Config(
        opcua_endpoint=str(opc.get("endpoint", "")),
        opcua_username=str(opc.get("username", "")),
        opcua_password=str(opc.get("password", "")),
        device_token=str(ac.get("deviceToken", "")),
        mqtt_host=str(ac.get("mqttHost", "mqtt.autoconnecto.in")),
        mqtt_port=int(ac.get("mqttPort", 8883)),
        ca_file=ca,
        insecure_tls=bool(ac.get("insecureTls", False)),
        poll_interval_ms=int(raw.get("pollIntervalMs", 1000)),
        mapping=mapping,
    )


def make_mqtt(cfg: Config) -> mqtt.Client:
    client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2,
        client_id=cfg.device_token,
    )
    client.username_pw_set(cfg.device_token, None)

    if cfg.insecure_tls:
        client.tls_set(cert_reqs=ssl.CERT_NONE)
        client.tls_insecure_set(True)
    else:
        if not cfg.ca_file.is_file():
            raise SystemExit(f"CA bundle not found: {cfg.ca_file}")
        client.tls_set(ca_certs=str(cfg.ca_file))

    return client


async def run(cfg: Config) -> None:
    if not cfg.opcua_endpoint:
        raise SystemExit("config.opcua.endpoint is required")
    if not cfg.device_token:
        raise SystemExit("config.autoconnecto.deviceToken is required")

    mqttc = make_mqtt(cfg)

    print(f"[MQTT] Connecting {cfg.mqtt_host}:{cfg.mqtt_port} ...")
    mqttc.connect(cfg.mqtt_host, cfg.mqtt_port, keepalive=60)
    mqttc.loop_start()

    topic = f"devices/{cfg.device_token}/telemetry"

    try:
        while True:
            try:
                async with UaClient(url=cfg.opcua_endpoint) as uac:
                    if cfg.opcua_username:
                        uac.set_user(cfg.opcua_username)
                        uac.set_password(cfg.opcua_password)

                    nodes = [
                        uac.get_node(m.node_id) for m in cfg.mapping
                    ]

                    print("[OPCUA] Connected")

                    while True:
                        values = await asyncio.gather(
                            *[n.read_value() for n in nodes],
                            return_exceptions=True,
                        )

                        payload: Dict[str, Any] = {
                            "ts": int(time.time() * 1000),
                        }

                        for m, v in zip(cfg.mapping, values):
                            if isinstance(v, Exception):
                                continue
                            payload[m.key] = _coerce_json(v)

                        data = json.dumps(payload).encode("utf-8")
                        info = mqttc.publish(topic, data, qos=1)
                        if info.rc != mqtt.MQTT_ERR_SUCCESS:
                            print(f"[MQTT] publish error rc={info.rc}")

                        await asyncio.sleep(cfg.poll_interval_ms / 1000.0)
            except Exception as exc:
                print(f"[OPCUA] error: {exc.__class__.__name__}: {exc}")
                await asyncio.sleep(2.0)
    finally:
        mqttc.loop_stop()
        mqttc.disconnect()


def main() -> int:
    if len(sys.argv) != 2:
        print("Usage: python opcua_gateway_mqtt.py config.json", file=sys.stderr)
        return 2

    cfg_path = Path(sys.argv[1]).resolve()
    cfg = load_config(cfg_path)
    asyncio.run(run(cfg))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

