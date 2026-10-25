"""MQTT topic helpers — matches AutoconnectoSDK MQTTTransport."""


def device_topic(device_token: str, suffix: str) -> str:
    return f"devices/{device_token}/{suffix}"


def telemetry_topic(device_token: str) -> str:
    return device_topic(device_token, "telemetry")


def client_attributes_topic(device_token: str) -> str:
    return device_topic(device_token, "attributes/client")


def shared_attributes_topic(device_token: str) -> str:
    return device_topic(device_token, "attributes/shared")


def shared_attributes_response_topic(device_token: str) -> str:
    return device_topic(device_token, "attributes/shared/response")


def shared_attributes_request_topic(device_token: str) -> str:
    return device_topic(device_token, "attributes/shared/request")


def rpc_request_subscribe(device_token: str) -> str:
    return device_topic(device_token, "rpc/request/+")


def rpc_response_topic(device_token: str, request_id: str) -> str:
    return device_topic(device_token, f"rpc/response/{request_id}")
