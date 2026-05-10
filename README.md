# Autoconnecto Arduino SDK

**Version:** 1.0.1  
**Platform:** Autoconnecto IoT  
**Target:** ESP32 (all variants)  
**Transport:** MQTT over WSS (primary) / MQTTS (fallback)

---

## What this SDK does

The Autoconnecto SDK connects your ESP32 to the Autoconnecto IoT platform. Once connected, your device can:

- Send **telemetry** (sensor readings) that appear as realtime charts on your dashboard
- Receive **shared attributes** from the dashboard (e.g. switch state, slider value, voltage limit) and apply them to hardware
- Send **client attributes** back to confirm the applied state — this enables the power-cycle sync mechanism
- Receive **RPC commands** from the dashboard (e.g. reboot, get status, set relay) and reply with a result
- Reconnect automatically after WiFi or broker drops

---

## How the attribute feedback loop works (platform USP)

This is what makes Autoconnecto different from a plain MQTT broker:

```
Dashboard user changes Switch widget
        ↓
Platform writes shared attribute  →  device/token/attributes/shared
        ↓
Device receives it via onAttributeUpdate()
        ↓
Device applies value to hardware (relay ON/OFF)
        ↓
Device sends client attribute confirmation  →  device/token/attributes/client
        ↓
Platform stores client attribute
        ↓
Dashboard reads client attribute — confirms hardware state
```

**On power cycle:** the device calls `sdk.requestSharedAttributes()` at startup. The platform sends back all stored shared attributes. `onAttributeUpdate()` fires for each one — the device applies them and sends confirmations. The dashboard stays in sync without any user action.

This solves the stale-state problem that exists in ThingsBoard and other platforms.

---

## Prerequisites

### Arduino IDE
Download from [arduino.cc](https://www.arduino.cc/en/software). Version 2.x recommended.

### ESP32 Board Support
In Arduino IDE: **File → Preferences → Additional boards manager URLs**, add:
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```
Then: **Tools → Board → Boards Manager** → search `esp32` → install **esp32 by Espressif Systems**.

### Required Libraries
Install from **Sketch → Include Library → Manage Libraries**:

| Library | Author | Purpose |
|---|---|---|
| ArduinoJson | Benoit Blanchon | JSON parsing for attributes and RPC |

The SDK uses the ESP-IDF native MQTT client (built into ESP32 Arduino core) — no additional MQTT library needed.

---

## Installation

### Option A — Directory Junction (recommended for development)

Run once in PowerShell (Windows):
```powershell
cmd /c mklink /J "$env:USERPROFILE\Documents\Arduino\libraries\AutoconnectoSDK" "C:\path\to\autoconnecto\sdk"
```

Any SDK changes you make are immediately visible to Arduino IDE. Restart Arduino IDE after creating the junction.

### Option B — Copy folder

Copy the entire `sdk/` folder to:
```
Windows:  Documents\Arduino\libraries\AutoconnectoSDK\
macOS:    ~/Documents/Arduino/libraries/AutoconnectoSDK/
Linux:    ~/Arduino/libraries/AutoconnectoSDK/
```

Restart Arduino IDE.

---

## Quick Start

```cpp
#include <AutoconnectoSDK.h>

AutoconnectoSDK sdk;

// Called when dashboard writes a shared attribute
void onAttributeUpdate(const String& key, float value) {
  if (key == "channel1") {
    digitalWrite(RELAY_PIN, value > 0.5 ? HIGH : LOW);
    sdk.sendClientAttribute("channel1", value > 0.5 ? 1.0 : 0.0); // confirm back
  }
}

// Called when an RPC command arrives from the dashboard
void onRPC(const String& method, JsonObject payload) {
  if (method == "reboot") {
    StaticJsonDocument<64> reply;
    reply["success"] = true;
    sdk.replyRPC(reply);
    delay(1000);
    ESP.restart();
  }
}

void setup() {
  SDKConfig config;
  config.wifiSSID        = "YOUR_WIFI";
  config.wifiPassword    = "YOUR_PASSWORD";
  config.mqttHost        = "mqtt.autoconnecto.in";
  config.mqttPort        = 8883;
  config.wssPort         = 8084;
  config.deviceToken     = "YOUR_DEVICE_TOKEN";
  config.enableWS        = true;
  config.enableMQTT      = true;
  config.allowInsecureTLS = false;
  config.rootCA          = AUTOCONNECTO_ROOT_CA; // see examples

  sdk.onAttributeUpdate(onAttributeUpdate);
  sdk.onRPC(onRPC);
  sdk.begin(config);
  sdk.requestSharedAttributes(); // restore state after power cycle
}

void loop() {
  sdk.loop(); // must be called frequently — never delay() > 50ms
  sdk.sendTelemetry("temperature", 25.4);
  delay(10);
}
```

---

## Configuration reference (`SDKConfig`)

| Field | Type | Required | Description |
|---|---|---|---|
| `wifiSSID` | `String` | Yes | WiFi network name |
| `wifiPassword` | `String` | Yes | WiFi password |
| `mqttHost` | `String` | Yes | Broker hostname e.g. `mqtt.autoconnecto.in` |
| `mqttPort` | `int` | Yes | MQTTS port, usually `8883` |
| `wssPort` | `int` | Yes | MQTT-over-WSS port, usually `8084` |
| `deviceToken` | `String` | Yes | Device token from Autoconnecto platform |
| `enableWS` | `bool` | No | Use WSS as primary transport (default `true`) |
| `enableMQTT` | `bool` | No | Use MQTTS as fallback (default `true`) |
| `allowInsecureTLS` | `bool` | No | Skip cert verification — only for local dev |
| `rootCA` | `const char*` | No | PEM root CA certificate for TLS verification |
| `enableSerialLogs` | `bool` | No | Print SDK logs to Serial |

---

## API reference

### Lifecycle

```cpp
sdk.begin(config);       // connect WiFi, start MQTT, register callbacks
sdk.loop();              // must run every iteration — handles keepalive and incoming messages
bool ok = sdk.connected(); // true if transport is active
```

### Telemetry

```cpp
// Single key
sdk.sendTelemetry("temperature", 25.4f);

// Two keys in one publish
sdk.sendTelemetry("humidity", 65.0f, "voltage", 230.0f);

// Three keys in one publish
sdk.sendTelemetry("current", 2.1f, "power", 480.0f, "frequency", 50.0f);

// Four or more keys — use JsonDocument
StaticJsonDocument<256> doc;
doc["voltage1"] = 230.0f;
doc["voltage2"] = 231.0f;
doc["voltage3"] = 229.0f;
sdk.sendTelemetry(doc);
```

### Client Attributes (device → platform)

```cpp
// Single value (float only)
sdk.sendClientAttribute("firmwareVersion", 1.01f);

// Multiple values
StaticJsonDocument<128> attrs;
attrs["freeHeap"] = ESP.getFreeHeap();
attrs["uptime"]   = millis() / 1000;
sdk.sendClientAttributes(attrs);
```

### Shared Attributes (platform → device)

```cpp
// Request all shared attributes (call at startup and on reconnect)
sdk.requestSharedAttributes();

// Request specific keys only
sdk.requestSharedAttributes("channel1,channel2,slider1");
```

Responses arrive via `onAttributeUpdate()` callback.

### RPC

```cpp
// Reply to an RPC call with a JSON document
StaticJsonDocument<128> reply;
reply["success"] = true;
reply["value"]   = 42;
sdk.replyRPC(reply);

// Reply with a simple boolean
sdk.replyRPC(true);
```

Always call `sdk.replyRPC()` inside your `onRPC` handler — even for unknown methods. Never drop an RPC silently.

### Transport status

```cpp
String transport = sdk.activeTransport(); // "WSS" or "MQTTS" or ""
bool usingWSS    = sdk.isUsingWSS();
bool usingMQTTS  = sdk.isUsingMQTTS();
```

---

## Callbacks

```cpp
// Shared attribute received (from dashboard or requestSharedAttributes response)
sdk.onAttributeUpdate([](const String& key, float value) {
  // apply to hardware, then confirm via sendClientAttribute
});

// RPC command received from dashboard
sdk.onRPC([](const String& method, JsonObject payload) {
  // payload contains: method, params, requestId
  // access params: payload["params"]["yourKey"]
  // always call sdk.replyRPC()
});

// Connection state changed
sdk.onConnect([](bool connected) {
  sdk.requestSharedAttributes(); // re-sync on every reconnect
});

sdk.onDisconnect([](bool connected) {
  // log or set LED indicator
});
```

---

## RPC payload format

When the dashboard sends an RPC command, the `onRPC` callback receives:

- `method` — the method name string (e.g. `"reboot"`, `"getStatus"`)
- `payload` — full JsonObject including `params` and `requestId`

```cpp
void onRPC(const String& method, JsonObject payload) {
  JsonObject params = payload["params"].as<JsonObject>();

  if (method == "setValue") {
    const char* key = params["key"] | "";
    float val       = params["value"] | 0.0f;
    sdk.sendClientAttribute(String(key), val);
    StaticJsonDocument<64> reply;
    reply["success"] = true;
    sdk.replyRPC(reply);
  }
}
```

---

## Critical rules

1. **Call `sdk.loop()` every iteration.** Never `delay()` more than ~50ms in the main loop. Long delays starve the MQTT keepalive and cause disconnects.

2. **Always call `sdk.replyRPC()`.** If an RPC has no reply the dashboard times out waiting. Reply with `{"success": false, "message": "Unknown method"}` for unhandled methods.

3. **Confirm every shared attribute via `sendClientAttribute()`.** The dashboard reads client attributes to confirm hardware state. Without confirmation, Switch/Slider widgets show stale state.

4. **Call `sdk.requestSharedAttributes()` on connect and reconnect.** This rebuilds device state from the platform after power cycles and reconnects.

---

## Examples

All examples use the **same keys** — one dashboard configuration works across all of them.

### `BasicTelemetry`
`examples/BasicTelemetry/BasicTelemetry.ino`

Start here. Connect to the platform and send telemetry every 10 seconds. No attributes, no RPC. Confirms your device token and network connectivity.

### `SwitchControl`
`examples/SwitchControl/SwitchControl.ino`

Demonstrates the full attribute feedback loop:
- 4-channel relay control via Switch widget (`channel1`–`channel4`)
- Slider/volume control via SliderControl widget (`volume`)
- Voltage limit control via AttributeControlCard widget (`limitVoltage1/2/3` → `setVoltage1/2/3`)
- Power-cycle sync via `requestSharedAttributes()` at startup

### `RPCCommands`
`examples/RPCCommands/RPCCommands.ino`

Demonstrates how to handle RPC commands from the dashboard RPC Widget:
- `ping`, `getStatus`, `getDiagnostics`, `relay_set`, `setValue`, `reset`, `reboot`, `telemetry_burst`
- Correct `replyRPC()` pattern for all cases including unknown methods

### `AllFunctionTest`
`examples/AllFunctionTest/AllFunctionTest.ino`

Production-grade reference sketch combining all of the above. Use this as a base for real device firmware.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Can't compile — `AutoconnectoSDK.h` not found | Library not installed | See Installation section |
| Connects but attributes not updating | `delay()` too long in loop | Replace with `delay(10)` |
| Switch widget shows stale state after device reboot | Not calling `requestSharedAttributes()` at startup | Add `sdk.requestSharedAttributes()` in `setup()` after `sdk.begin()` |
| RPC times out on dashboard | Not calling `sdk.replyRPC()` | Always reply, even for unknown methods |
| TLS error at connection | Wrong root CA or insecure broker | Set `config.allowInsecureTLS = true` for local testing |
| Device disconnects every ~60s | MQTT keepalive not being serviced | Ensure `sdk.loop()` runs at least every 50ms |

---

## MQTT topic map (internal reference)

| Direction | Topic | Purpose |
|---|---|---|
| Device → Platform | `devices/{token}/telemetry` | Sensor readings |
| Device → Platform | `devices/{token}/attributes/client` | Client attribute updates |
| Device → Platform | `devices/{token}/attributes/shared/request` | Request shared attributes |
| Device → Platform | `devices/{token}/rpc/response/{requestId}` | RPC reply |
| Platform → Device | `devices/{token}/attributes/shared` | Live shared attribute push |
| Platform → Device | `devices/{token}/attributes/shared/response` | Response to attribute request |
| Platform → Device | `devices/{token}/rpc/request/{requestId}` | Incoming RPC command |
