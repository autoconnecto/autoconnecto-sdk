# LTE PPP (Quectel EC200) — optional

WiFi examples and the default library build are **unchanged**. LTE is **opt-in** at compile time.

## Requirements

- **ESP32 Arduino core 3.x** (built-in `PPP` library — no TinyGSM)
- Sketch folder **`build_opt.h`** with `-DAUTOCONNECTO_ENABLE_LTE_PPP=1`
- Quectel **EC200** (or compatible) on UART, SIM with known **APN**

## Wiring (default in `SDKConfig`)

| ESP32 | Modem |
|-------|--------|
| GPIO **16** | RX ← modem TX |
| GPIO **17** | TX → modem RX |
| Optional | RESET / PWR_KEY per your breakout |

**Warning:** GPIO 16/17 are **PSRAM pins** on some ESP32-WROVER modules. Use a WROOM board or adjust `lteUartRx` / `lteUartTx`.

## Sketch configuration

```cpp
config.networkMode = NetworkMode::LtePpp;
config.lteApn      = "your.carrier.apn";
config.lteUartRx   = 16;
config.lteUartTx   = 17;
config.lteResetPin = 5;   // -1 if not wired
// WiFi fields ignored when networkMode is LtePpp
```

MQTT uses **MQTTS :8883** (not WSS) on LTE.

## Example

`examples/AllFunctionTest_lte_ppp_mqtt/` — same AllFunctions behaviour as `AllFunctionTest_mqtt`, LTE bring-up.

## Adding other modems later

Change `PPP_MODEM_EC200` in `src/network/LtePppNetwork.cpp` (or add a `SDKConfig` modem profile) and re-test attach + PPP + MQTTS. WiFi builds are unaffected.
