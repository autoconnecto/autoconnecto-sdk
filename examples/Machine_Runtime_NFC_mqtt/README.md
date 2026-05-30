# Machine_Runtime_NFC_mqtt (Rev 2)

**Contract:** [`MACHINE_RUNTIME_NFC.md`](../../MACHINE_RUNTIME_NFC.md)  
**BOM:** Rev 2.0 — [Mifra PN532 MT0359](https://mifraelectronics.com/product/pn532-nfc-rfid-read-write-module/) (I2C, GPIO 21/22)

## Sketches

| File | Purpose |
|------|---------|
| `Machine_Runtime_NFC_mqtt.ino` | Production — read card id+name, PZEM, MQTT, SSR |
| [`../Machine_Runtime_NFC_enroll/`](../Machine_Runtime_NFC_enroll/) | Office — write `employee_id` + `display_name` to new cards |

## Arduino libraries

Install via **Library Manager**:

1. **Adafruit PN532**
2. **Adafruit BusIO**
3. **AutoconnectoSDK** (this repo)

## Wiring (frozen)

| PN532 | ESP32 |
|-------|-------|
| SDA | 21 |
| SCL | 22 |
| IRQ | 4 |
| RST | 5 |
| VCC | 5 V |
| GND | GND |

**Module DIP switch:** **I2C** (not SPI/UART).

PZEM UART2: RX=16, TX=17. Fotek SSR: GPIO 26.

## First-time setup

1. Wire PN532 in **I2C** mode.
2. Flash **`Machine_Runtime_NFC_enroll`** sketch on a desk ESP32 (separate folder).
3. Serial Monitor 115200: `w EMP1042 Rajesh Kumar` (card on reader).
4. Verify: `r`
5. Flash **`Machine_Runtime_NFC_mqtt.ino`** on machine ESP32; set WiFi + device token.
6. Tap card at machine → dashboard shows **Rajesh Kumar** / **EMP1042** with no platform worker list.

## Tap behaviour

- First tap (or different card): **session IN** — sends `machine_operator_id`, `machine_operator_name`, `machine_session_active=true`
- Second tap **same card**: **session OUT**
- Card without enrollment magic `ACMRUNv1`: rejected (serial message)

## Card layout

| Block | Content |
|-------|---------|
| 4 | `ACMRUNv1` |
| 5 | employee_id (16 chars) |
| 6 | display_name (32 chars) |

Default MIFARE key A: `FF FF FF FF FF FF`

## Pilot workers (examples)

| employee_id | display_name |
|-------------|----------------|
| EMP001 | worker1 |
| EMP002 | worker2 |
| EMP003 | worker3 |
| EMP004 | worker4 |

Write each with enroll sketch; issue one card per worker.
