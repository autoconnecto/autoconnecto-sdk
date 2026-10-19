# LTE PPP (Quectel EC200) — optional

WiFi examples and the default library build are **unchanged**. LTE is **opt-in** at compile time.

## Requirements

- **ESP32 Arduino core 3.x** (built-in `PPP` library — no TinyGSM)
- Sketch folder **`build_opt.h`** with one line: `-DAUTOCONNECTO_ENABLE_LTE_PPP=1`. A `//` comment in that file is passed to the compiler as a flag and the build fails.
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

ESP32 core 3.3 has no EC200 model id. The sketch uses `PPP_MODEM_GENERIC`, which speaks the standard AT set the EC200 uses for attach and PPP. A modem on the bench is what proves the link. Compiling is the check that the names exist.
