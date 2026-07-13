# Machine runtime — NFC worker identity (Rev 2)

**Status:** FROZEN (hardware reader + card type)  
**BOM:** [`MACHINE_RUNTIME_HARDWARE_BOM.md`](MACHINE_RUNTIME_HARDWARE_BOM.md) § Rev 2.0  
**Firmware (pilot, not in SDK yet):** [`tools/machine-runtime-nfc/Machine_Runtime_NFC_mqtt/`](../tools/machine-runtime-nfc/Machine_Runtime_NFC_mqtt/) (machine) · [`tools/machine-runtime-nfc/Machine_Runtime_NFC_enroll/`](../tools/machine-runtime-nfc/Machine_Runtime_NFC_enroll/) (desk enrollment)

Rev 2 replaces **125 kHz EM4100** (UID-only) with **13.56 MHz NFC** so each badge carries **employee id + display name on the card**. The platform does **not** maintain a per-dashboard worker name list for production.

---

## Frozen reader (India)

| Field | Value |
|-------|--------|
| **Product** | PN532 NFC RFID Read / Write Module |
| **Supplier** | [Mifra Electronics](https://mifraelectronics.com/product/pn532-nfc-rfid-read-write-module/) |
| **SKU** | MT0359 |
| **Price guide** | ~₹398 (check site; was ~~₹590~~) |
| **Support** | +91 8073744810 / support@mifratech.com |
| **Frequency** | 13.56 MHz |
| **Range** | ~5–7 cm (on-board antenna) |
| **Interfaces** | I2C, SPI, UART (HSU) — **use I2C on machine** |
| **Level** | On-board shifter: 5 V TTL I2C/UART; **3.3 V SPI** |

**Module mode switch:** set to **I2C** at the factory / first install.

---

## Frozen cards

| Field | Value |
|-------|--------|
| **Type** | **MIFARE Classic 1K** (ISO PVC card, 13.56 MHz) |
| **Not compatible** | 125 kHz EM4100, T5577 as EM4100 replacement |

**Order:** “MIFARE Classic 1K card 13.56MHz” from same channels as other RFID (Robu, Amazon, Mifra). Printable PVC optional (name on plastic + data in chip).

---

## Card memory layout (frozen)

Written once at **enrollment**; ESP32 only **reads** at the machine.

| Block | Content | Max length | Example |
|-------|---------|------------|---------|
| Sector 1 block 0 | Magic | 8 bytes | `ACMRUNv1` |
| Sector 1 block 1 | **employee_id** | 16 ASCII | `EMP1042` |
| Sector 1 block 2 | **display_name** | 32 ASCII | `Rajesh K` |

**Rules:**

- ASCII, null-padded; no UTF-8 requirement in Rev 2.  
- If read fails → tap rejected; log on serial; do not invent names in firmware.

---

## Telemetry (device → platform)

On tap IN / OUT (same session rules as Rev 1):

| Key | Source | Example |
|-----|--------|---------|
| `machine_operator_id` | From card `employee_id` | `EMP1042` |
| `machine_operator_name` | From card `display_name` | `Rajesh K` |
| `machine_session_active` | ESP session state | `true` / `false` |
| `machine_current_a` | PZEM | unchanged |

**No platform worker registry required** for display when cards are programmed correctly.

---

## ESP32 wiring (Rev 2 — I2C)

| PN532 | ESP32 | Notes |
|-------|-------|--------|
| SDA | **GPIO 21** | I2C data |
| SCL | **GPIO 22** | I2C clock |
| IRQ | **GPIO 4** | Optional, recommended |
| RST | **GPIO 5** | Optional |
| VCC | **5 V** | From machine 5 V SMPS |
| GND | GND | Common with ESP32 / PZEM |

**Unchanged from Rev 1:** PZEM UART2 (16/17), Fotek SSR (26).

**Tap pod:** PN532 module behind **plastic** face (5–7 cm tap). Reader not behind metal.

**Cable:** 4-core 0.5 mm² (VCC, GND, SDA, SCL) + optional IRQ/RST, **≤ 1 m** to main box.

---

## Enrollment (office)

| Item | Spec |
|------|------|
| Reader | Same **PN532** module (or USB PN532 on PC) |
| Tool | **`tools/machine-runtime-nfc/Machine_Runtime_NFC_enroll`** — serial `w worker1 Rajesh Kumar` |
| Process | New worker → write card → hand card → **no dashboard edit** |
| Worker leaves | Collect card; issue newly written card |

**Pilot:** Android “MIFARE Classic Tool” + PN532 can prove writes until enrollment firmware ships.

---

## Production vs Rev 1

| | Rev 1 | Rev 2 |
|---|-------|-------|
| Reader | 125 kHz EM4100 UART | **PN532 I2C** ([Mifra MT0359](https://mifraelectronics.com/product/pn532-nfc-rfid-read-write-module/)) |
| Card | UID only | **id + name on card** |
| Platform worker list | Was considered | **Not required** |
| Sketch | `Machine_Runtime_mqtt.ino` | `Machine_Runtime_NFC_mqtt.ino` |

Existing Rev 1 pilots may stay on EM4100 until hardware swap; new machines use Rev 2.
