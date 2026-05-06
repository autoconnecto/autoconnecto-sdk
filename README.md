# Autoconnecto Arduino SDK

Version: 1.0.1

Production-ready ESP32 MQTT SDK for Autoconnecto IoT Platform.

## Features

- Telemetry
- Client Attributes
- Shared Attributes
- Commands
- ACK
- Auto Reconnect
- Online Status

## Supported Boards

- ESP32 DevKit
- NodeMCU-32S
- ESP32 compatible boards

## Dependencies

Install from Arduino Library Manager:

### PubSubClient
Author: Nick O'Leary

### ArduinoJson
Author: Benoit Blanchon

### ESP32 Boards
Author: Espressif Systems

## Installation

Copy folder to:

Documents/Arduino/libraries/AutoconnectoSDK

Restart Arduino IDE.

## Examples

### BasicTelemetry
Quick first success.

### FullParitySingleDevice
Production parity example.

### AllFunctions
Every public SDK method demonstrated.

## Quick Start

```cpp
AutoconnectoSDK sdk;

sdk.setWiFi("WIFI","PASS");
sdk.setMQTT("BROKER_IP",1883);
sdk.setToken("DEVICE_TOKEN");

sdk.begin();