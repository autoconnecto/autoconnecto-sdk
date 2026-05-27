"""MQTT device client — MQTTS, same topics as AutoconnectoSDK."""

from __future__ import annotations

import json
import ssl
import time
from typing import Any, Callable, Dict, Optional

import paho.mqtt.client as mqtt

from .config import Settings
from .topics import (
    client_attributes_topic,
    rpc_request_subscribe,
    rpc_response_topic,
    shared_attributes_request_topic,
    shared_attributes_response_topic,
    shared_attributes_topic,
    telemetry_topic,
)


class AutoconnectoMqttDevice:
    def __init__(self, settings: Settings) -> None:
        self.settings = settings
        self._token = settings.device_token
        self._last_rpc_id: str = ""
        self._on_attribute: Optional[Callable[[str, float], None]] = None
        self._on_rpc: Optional[
            Callable[[str, Dict[str, Any]], None]
        ] = None
        self._on_connect_cb: Optional[Callable[[], None]] = None

        self.client = mqtt.Client(
            mqtt.CallbackAPIVersion.VERSION2,
            client_id=self._token,
        )
        self.client.username_pw_set(self._token, None)
        self.client.on_connect = self._on_connect
        self.client.on_message = self._on_message

        if settings.insecure_tls:
            self.client.tls_set(cert_reqs=ssl.CERT_NONE)
            self.client.tls_insecure_set(True)
        else:
            self.client.tls_set(ca_certs=str(settings.ca_file))

    def on_attribute_update(
        self, cb: Callable[[str, float], None]
    ) -> None:
        self._on_attribute = cb

    def on_rpc(
        self, cb: Callable[[str, Dict[str, Any]], None]
    ) -> None:
        self._on_rpc = cb

    def on_connected(self, cb: Callable[[], None]) -> None:
        self._on_connect_cb = cb

    def connect(self) -> None:
        print(
            f"[MQTT] Connecting to {self.settings.mqtt_host}:"
            f"{self.settings.mqtt_port} ..."
        )
        self.client.connect(
            self.settings.mqtt_host,
            self.settings.mqtt_port,
            keepalive=60,
        )
        self.client.loop_start()

    def disconnect(self) -> None:
        self.client.loop_stop()
        self.client.disconnect()

    def request_shared_attributes(self, keys: str = "") -> None:
        body: Dict[str, Any] = {"requestId": int(time.time() * 1000)}
        if keys:
            body["keys"] = keys
        self._publish(
            shared_attributes_request_topic(self._token),
            json.dumps(body),
        )

    def send_telemetry(self, payload: Dict[str, Any]) -> None:
        self._publish(
            telemetry_topic(self._token),
            json.dumps(payload),
        )

    def send_telemetry_value(self, key: str, value: float) -> None:
        self.send_telemetry({key: value})

    def send_client_attributes(self, attrs: Dict[str, Any]) -> None:
        self._publish(
            client_attributes_topic(self._token),
            json.dumps(attrs),
        )

    def send_client_attribute(self, key: str, value: float) -> None:
        self.send_client_attributes({key: value})

    def reply_rpc(self, body: Dict[str, Any]) -> bool:
        if not self._last_rpc_id:
            print("[RPC] No request id — cannot reply")
            return False
        topic = rpc_response_topic(self._token, self._last_rpc_id)
        self._publish(topic, json.dumps(body))
        return True

    def _publish(self, topic: str, payload: str) -> None:
        info = self.client.publish(topic, payload, qos=1)
        if info.rc != mqtt.MQTT_ERR_SUCCESS:
            print(f"[MQTT] publish failed rc={info.rc} topic={topic}")

    def _on_connect(
        self,
        client: mqtt.Client,
        userdata: Any,
        flags: mqtt.ConnectFlags,
        reason_code: mqtt.ReasonCode,
        properties: Any = None,
    ) -> None:
        if reason_code != 0:
            print(f"[MQTT] connect failed: {reason_code}")
            return
        print("[MQTT] Connected (MQTTS)")
        client.subscribe(shared_attributes_topic(self._token), qos=1)
        client.subscribe(
            shared_attributes_response_topic(self._token), qos=1
        )
        client.subscribe(rpc_request_subscribe(self._token), qos=1)
        self.request_shared_attributes()
        if self._on_connect_cb:
            self._on_connect_cb()

    def _on_message(
        self, client: mqtt.Client, userdata: Any, msg: mqtt.MQTTMessage
    ) -> None:
        topic = msg.topic
        try:
            payload = msg.payload.decode("utf-8")
            doc = json.loads(payload)
        except (UnicodeDecodeError, json.JSONDecodeError) as exc:
            print(f"[MQTT] bad message on {topic}: {exc}")
            return

        if topic.endswith("/attributes/shared") or topic.endswith(
            "/attributes/shared/response"
        ):
            if isinstance(doc, dict) and self._on_attribute:
                for key, val in doc.items():
                    try:
                        self._on_attribute(key, float(val))
                    except (TypeError, ValueError):
                        pass
            return

        if "/rpc/request/" in topic:
            rid = topic.rsplit("/", 1)[-1]
            self._last_rpc_id = rid
            method = str(doc.get("method", ""))
            if self._on_rpc:
                self._on_rpc(method, doc)
