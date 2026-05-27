#!/usr/bin/env python3
"""Modbus gateway -> Autoconnecto telemetry over MQTTS.

Supports:
- Modbus TCP
- Modbus RTU (serial)
"""

from __future__ import annotations

import asyncio
import json
import ssl
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, List, Optional

import paho.mqtt.client as mqtt
from pymodbus.client import AsyncModbusSerialClient, AsyncModbusTcpClient


def _coerce_num(v: Any, scale: float, signed: bool) -> float:
    try:
        n = int(v)
    except Exception:
        return 0.0
    # signed handling for 16-bit
    if signed and n > 32767:
        n = n - 65536
    return float(n) * scale


@dataclass(frozen=True)
class MapItem:
    key: str
    reg_type: str
    address: int
    count: int
    scale: float
    signed: bool


@dataclass
class Config:
    mode: str
    tcp_host: str
    tcp_port: int
    rtu_port: str
    rtu_baud: int
    rtu_parity: str
    rtu_stopbits: int
    rtu_bytesize: int
    unit_id: int
    device_token: str
    mqtt_host: str
    mqtt_port: int
    ca_file: Path
    insecure_tls: bool
    poll_interval_ms: int
    mapping: List[MapItem]


def load_config(path: Path) -> Config:
    raw = json.loads(path.read_text(encoding="utf-8"))
    mb = raw.get("modbus") or {}
    tcp = mb.get("tcp") or {}
    rtu = mb.get("rtu") or {}
    ac = raw.get("autoconnecto") or {}
    mapping_raw = raw.get("mapping") or []
    mapping = [
        MapItem(
            key=str(m.get("key", "")),
            reg_type=str(m.get("type", "")),
            address=int(m.get("address", 0)),
            count=int(m.get("count", 1)),
            scale=float(m.get("scale", 1.0)),
            signed=bool(m.get("signed", False)),
        )
        for m in mapping_raw
        if isinstance(m, dict) and m.get("key") and m.get("type")
    ]
    if not mapping:
        raise SystemExit("config.mapping must contain at least one item")

    return Config(
        mode=str(mb.get("mode", "tcp")).lower(),
        tcp_host=str(tcp.get("host", "127.0.0.1")),
        tcp_port=int(tcp.get("port", 502)),
        rtu_port=str(rtu.get("port", "/dev/ttyUSB0")),
        rtu_baud=int(rtu.get("baud", 9600)),
        rtu_parity=str(rtu.get("parity", "N")),
        rtu_stopbits=int(rtu.get("stopbits", 1)),
        rtu_bytesize=int(rtu.get("bytesize", 8)),
        unit_id=int(mb.get("unitId", 1)),
        device_token=str(ac.get("deviceToken", "")),
        mqtt_host=str(ac.get("mqttHost", "mqtt.autoconnecto.in")),
        mqtt_port=int(ac.get("mqttPort", 8883)),
        ca_file=Path(str(ac.get("caFile", ""))).resolve(),
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


async def poll_once(client, cfg: Config) -> Dict[str, Any]:
    payload: Dict[str, Any] = {"ts": int(time.time() * 1000)}

    for m in cfg.mapping:
        try:
            if m.reg_type == "holding":
                rr = await client.read_holding_registers(
                    m.address, m.count, slave=cfg.unit_id
                )
                regs = rr.registers if rr and not rr.isError() else []
            elif m.reg_type == "input":
                rr = await client.read_input_registers(
                    m.address, m.count, slave=cfg.unit_id
                )
                regs = rr.registers if rr and not rr.isError() else []
            else:
                regs = []
            if regs:
                payload[m.key] = _coerce_num(regs[0], m.scale, m.signed)
        except Exception:
            continue

    return payload


async def run(cfg: Config) -> None:
    if not cfg.device_token:
        raise SystemExit("config.autoconnecto.deviceToken is required")

    mqttc = make_mqtt(cfg)
    mqttc.connect(cfg.mqtt_host, cfg.mqtt_port, keepalive=60)
    mqttc.loop_start()

    topic = f"devices/{cfg.device_token}/telemetry"

    try:
        while True:
            try:
                if cfg.mode == "rtu":
                    mbc = AsyncModbusSerialClient(
                        port=cfg.rtu_port,
                        baudrate=cfg.rtu_baud,
                        parity=cfg.rtu_parity,
                        stopbits=cfg.rtu_stopbits,
                        bytesize=cfg.rtu_bytesize,
                    )
                else:
                    mbc = AsyncModbusTcpClient(cfg.tcp_host, port=cfg.tcp_port)

                await mbc.connect()
                if not mbc.connected:
                    raise RuntimeError("Modbus connect failed")

                print(f"[MODBUS] Connected ({cfg.mode})")

                while True:
                    payload = await poll_once(mbc, cfg)
                    data = json.dumps(payload).encode("utf-8")
                    mqttc.publish(topic, data, qos=1)
                    await asyncio.sleep(cfg.poll_interval_ms / 1000.0)
            except Exception as exc:
                print(f"[MODBUS] error: {exc.__class__.__name__}: {exc}")
                await asyncio.sleep(2.0)
    finally:
        mqttc.loop_stop()
        mqttc.disconnect()


def main() -> int:
    if len(sys.argv) != 2:
        print("Usage: python modbus_gateway_mqtt.py config.json", file=sys.stderr)
        return 2
    cfg = load_config(Path(sys.argv[1]).resolve())
    asyncio.run(run(cfg))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

