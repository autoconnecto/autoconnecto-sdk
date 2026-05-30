# Machine Runtime — Frozen hardware BOM (Rev 1.0)

**Status:** FROZEN for pilot + 250-machine rollout  
**Date:** 2026-05-29  
**Contract (software):** [`MACHINE_RUNTIME.md`](MACHINE_RUNTIME.md)  
**Firmware sketch:** [`examples/Machine_Runtime_mqtt/Machine_Runtime_mqtt.ino`](examples/Machine_Runtime_mqtt/Machine_Runtime_mqtt.ino)

This document is the **single source of truth** for field hardware. Do not mix reader types, SSR types, or enclosure layouts without a **BOM Rev 2.0**.

---

## 1. System blocks (per machine)

| Block | Function |
|-------|----------|
| **ESP32** | MQTT, PZEM Modbus, RFID session, Fotek SSR interlock |
| **PZEM-004T** | `machine_current_a` (+ voltage available for future use) |
| **125 kHz RFID** | Worker tap IN / tap OUT (EM4100 cards) |
| **Fotek SSR** | Enable path: no session → open; session → closed |
| **5 V PSU** | ESP32 + RFID + SSR input (from 230 V AC panel) |
| **Tap pod** | Plastic enclosure for RFID at operator height |
| **Main box** | IP65 enclosure for ESP, PSU, SSR, RS485, terminals |

**Platform:** thresholds, KPIs, fleet widget, cycles (software — see `MACHINE_RUNTIME.md`).  
**Worker identity (Rev 1):** card UID only — platform map optional. **Rev 2:** id+name on card — see § Rev 2.0 below.

---

## 2. Per-machine BOM (frozen)

| # | Part | Spec (frozen) | Qty | ₹ unit (guide) | ₹ line |
|---|------|----------------|-----|----------------|--------|
| 1 | ESP32 DevKit | WROOM-32, 38-pin, USB for provisioning only | 1 | 350 | 350 |
| 2 | PZEM-004T | 100 A, Modbus RTU, addr **0xF8** | 1 | 650 | 650 |
| 3 | MAX485 module | 3.3 V logic, half-duplex | 1 | 80 | 80 |
| 4 | 5 V SMPS | 85–265 V AC in, **5 V 2 A** min, DIN or PCB mount | 1 | 300 | 300 |
| 5 | 125 kHz RFID reader | **Bare** EM4100, **UART 9600** (not Wiegand for Rev 1) | 1 | 250 | 250 |
| 6 | RFID tap pod | ABS/IP65, **plastic face** for tap; ~80×50×45 mm min | 1 | 150 | 150 |
| 7 | Main enclosure | IP65, glands, **metal OK** (reader not inside metal) | 1 | 400 | 400 |
| 8 | Fotek SSR | See §4 — **one per machine** | 1 | 250 | 250 |
| 9 | SSR heatsink | Fit Fotek size; required if enable ON for full shift | 1 | 80 | 80 |
| 10 | Cable reader → main | 4-core 0.5 mm², **≤ 1 m** (VCC, GND, TX, RX) | 1 | 60 | 60 |
| 11 | Terminal blocks + fuse | 5 V rail **1 A fuse**; mains input fused per electrician | 1 set | 120 | 120 |
| 12 | Consumables | Labels, ties, crimps, mounting screws | 1 set | 80 | 80 |
| | | | | **Typical total** | **~2,770** |

**Budget range:** ₹2,400 – 3,200 / machine (vendor + shipping).  
**× 250 machines:** ~₹6.0 – 8.0 lakh hardware (excludes install labour).

---

## 3. Per-worker items (not per machine)

| Part | Spec | Qty |
|------|------|-----|
| 125 kHz EM4100 card | Passive, factory-standardized UID | 1 per worker |
| Card holder / lanyard | Optional | 1 per worker |

**₹ guide:** ₹30 – 80 × headcount (e.g. 100 workers → ₹3k – 8k once).

---

## 4. Fotek SSR — frozen by install type

| Install code | Panel | Fotek type | Control | Load switched |
|--------------|-------|------------|---------|----------------|
| **INT-SSR-DC** | 24 V DC contactor coil or 24 V enable | **DC → DC** (e.g. SSR-25DD / 40DD class) | ESP32 GPIO → 3–32 V DC input | 24 V DC enable (electrician sizes) |
| **INT-SSR-AC** | 230 V AC contactor coil or AC enable | **DC → AC** (e.g. SSR-25DA / 40DA class) | ESP32 GPIO → 3–32 V DC input | 230 V AC enable (electrician sizes) |

**Rules (frozen):**

- SSR in series with **enable / coil control only** — never motor power.
- Heatsink on shift-long ON.
- User field experience: Fotek from ESP32 GPIO on contactor coils up to large contactors — **accepted standard for this project**.
- Fail mode: SSR may fail short ON — owner informed; badge interlock is primary policy control.

**Do not use:** bare 5 V 10 A blue relay modules on raw 230 V coil for production (pilot only if needed).

---

## 5. RFID — frozen

| Item | Frozen choice |
|------|----------------|
| Frequency | **125 kHz** |
| Cards | **EM4100** passive |
| Reader | **Bare module**, UART **9600 8N1** |
| Housed reader ₹2,500+ | **Not in BOM** |
| Mount | **Separate tap pod** (§2 #6), not behind metal |
| Tap cable | **≤ 1 m** to main box |

**Enrolment:** Read UID once per worker → HR / platform `card_uid` → `worker_id`.

---

## 6. Enclosures — frozen layout

```
┌─────────────────────┐     ≤1 m 4-core      ┌──────────────────────────┐
│  TAP POD (plastic)  │ ──────────────────── │  MAIN BOX (IP65)         │
│  125 kHz reader     │                      │  5 V SMPS                │
│  "TAP CARD HERE"    │                      │  ESP32                   │
└─────────────────────┘                      │  MAX485 + PZEM wiring    │
        @ operator height                    │  Fotek SSR + heatsink    │
                                             │  Terminals / fuses       │
                                             └──────────────────────────┘
                                                    @ panel / machine
```

**Frozen:** Reader in **tap pod**; ESP + SSR + PZEM in **main box**.  
**Allowed later (Rev 2):** Single box with **plastic lid window** only if same tap-pod rules met.

---

## 7. ESP32 wiring — frozen pinout (Rev 1 firmware)

| Signal | GPIO | Notes |
|--------|------|--------|
| PZEM Modbus RX | **16** | UART2 RX — matches current sketch |
| PZEM Modbus TX | **17** | UART2 TX |
| RFID UART RX | **18** | Reader TX → ESP RX |
| RFID UART TX | **19** | Reader RX → ESP TX (if reader needs TX) |
| Fotek SSR IN+ | **26** | 3.3 V GPIO; SSR IN− → GND |
| Status LED (opt.) | **2** | Boot-safe |

**Power:** Common GND (ESP, PSU, reader, SSR input). Reader VCC **5 V** from SMPS (≤ 200 mA budget).

**PZEM:** Modbus **9600**, slave **0xF8**. CT on **one phase** — electrician.

---

## 8. Telemetry & events (frozen keys)

| Key / event | Source | Purpose |
|-------------|--------|---------|
| `machine_current_a` | PZEM | Load classification, cycles |
| `machine_sensor_ok` | ESP | `false` if Modbus fails |
| `operator_session_start` | ESP (RFID) | Worker tap IN — **software TBD** |
| `operator_session_end` | ESP (RFID) | Worker tap OUT — **software TBD** |
| `machine_cycle_count` | ESP or platform | Work cycles — **software TBD** |

Thresholds (**Off / Idle / Load A**) remain **platform only** — not on device.

---

## 9. Install checklist (electrician)

1. Label machine: **INT-SSR-DC** or **INT-SSR-AC**.
2. Mount **tap pod** at operator tap height; **main box** in panel.
3. Wire Fotek output in **enable** path only; fuse per local code.
4. PZEM CT on one phase; RS485 A/B to MAX485.
5. 230 V AC → 5 V SMPS (separate from PZEM’s own AC supply).
6. Provisioning: flash `Machine_Runtime_mqtt.ino`, set device token, verify `machine_current_a` on device page.
7. Enrol worker cards; set thresholds in widget; **Save thresholds (platform)**.

---

## 10. Site-level (once per site, not per machine)

| Item | Qty |
|------|-----|
| Wi‑Fi AP / coverage in shop | Per zone |
| Spares kit | 5–10% ESP, reader, SSR, PSU |
| USB cable for flash | 2–3 |
| HR worker + card UID spreadsheet | 1 import |

---

## 11. Explicitly excluded from Rev 1 BOM

- Housed RFID reader (₹2,500–3,000)
- 24 V industrial coil relay (not needed — Fotek + 5 V PSU)
- MFRC522 13.56 MHz (wrong frequency for frozen standard)
- Mechanical relay module as primary interlock (Fotek is standard)
- Voltage-based “power present” logic (future)
- Second PSU 24 V in control box

---

## 12. Where to buy (India)

| Category | Channels |
|----------|----------|
| ESP32, MAX485, 5 V SMPS, bare RFID, tap pod, enclosures | Robu, Probots, Amazon, local electronics |
| PZEM-004T | Robu, Amazon, AliExpress bulk |
| Fotek SSR DA/DD + heatsink | Existing supplier / Amazon / industrial market |
| Bulk 250+ | Negotiate 5–10% on ESP + reader + enclosures |

---

## 13. Software scope (reference — not hardware)

| Feature | Status |
|---------|--------|
| Fleet widget, thresholds, `machine_current_a` | Built |
| Runtime site/area filter | Built |
| Email reports (Brevo) | Built |
| Worker sessions + RFID (Rev 1 EM4100) | Partial (`Machine_Runtime_mqtt.ino`) |
| NFC PN532 + id/name on card (Rev 2) | **Hardware frozen** — firmware planned |
| Cycle / product count | **Planned** |
| Lifetime worker productivity | **Planned** |

---

## 14. Revision history

| Rev | Date | Change |
|-----|------|--------|
| **1.0** | 2026-05-29 | Initial freeze: 125 kHz bare + tap pod, Fotek SSR, 5 V box, INT-SSR-DC/AC |

**Next change:** BOM Rev 2.1+ only with written approval.

---

# Machine Runtime — Frozen hardware BOM (Rev 2.0)

**Status:** FROZEN for **new** machines (worker NFC identity)  
**Date:** 2026-05-29  
**NFC contract:** [`MACHINE_RUNTIME_NFC.md`](MACHINE_RUNTIME_NFC.md)  
**Supersedes (RFID only):** Rev 1.0 §5 (125 kHz EM4100) — do not mix readers on one machine.

Rev 2 keeps **PZEM, Fotek SSR, enclosures, 5 V PSU, ESP32** from Rev 1. Only **worker RFID** changes.

---

## Rev 2 — system blocks (per machine)

| Block | Function |
|-------|----------|
| **ESP32** | MQTT, PZEM Modbus, **NFC session (PN532)**, Fotek SSR |
| **PZEM-004T** | `machine_current_a` (unchanged) |
| **PN532 NFC** | Read **employee id + name** from MIFARE card; tap IN/OUT |
| **Fotek SSR** | Unchanged |
| **5 V PSU** | ESP32 + **PN532** + SSR (budget **~250 mA** extra for NFC) |
| **Tap pod** | Plastic face; **13.56 MHz** coil — not metal |

**Platform:** thresholds, fleet widget, tool life — unchanged.  
**Worker names:** on **card**, not on platform config.

---

## Rev 2 — BOM changes vs Rev 1

| # | Part | Spec (frozen) | Qty | ₹ guide (unit) |
|---|------|----------------|-----|----------------|
| 5 | **PN532 NFC module** | [Mifra MT0359](https://mifraelectronics.com/product/pn532-nfc-rfid-read-write-module/) — **I2C mode**, 5–7 cm | 1 | **~398** |
| 6 | RFID tap pod | Same as Rev 1 (plastic, operator height) | 1 | ~150 |
| — | **125 kHz EM4100 reader** | **Remove** from new builds | 0 | — |

**Line items 1–4, 7–12:** same as Rev 1 (ESP32, PZEM, MAX485, SMPS, enclosure, Fotek, cable, terminals).

**Rev 2 typical total:** ~**2,920** / machine (≈ Rev 1 + ₹150 for PN532 vs bare 125 kHz reader).

---

## Rev 2 — per-worker items

| Part | Spec | Qty |
|------|------|-----|
| **MIFARE Classic 1K** ISO PVC card | 13.56 MHz, **programmed** with id + name | 1 per worker |
| Card holder / lanyard | Optional | 1 per worker |

**₹ guide:** ~₹20–45/card × headcount + **one-time** enrollment station (extra PN532 or USB reader).

**Not used on Rev 2:** 125 kHz EM4100 cards.

---

## Rev 2 — NFC reader (frozen)

| Item | Frozen choice |
|------|----------------|
| Module | **PN532** read/write, on-board antenna |
| Vendor (frozen) | **Mifra Electronics** — [PN532 NFC RFID Read / Write Module](https://mifraelectronics.com/product/pn532-nfc-rfid-read-write-module/) |
| SKU | **MT0359** |
| Frequency | **13.56 MHz** |
| Host interface | **I2C** (module switch = I2C) |
| Range | **5–7 cm** |
| Cards | **MIFARE Classic 1K** |
| Mount | Tap pod, plastic window |
| Cable to main | ≤ 1 m (5 V, GND, SDA, SCL; IRQ/RST optional) |

---

## Rev 2 — ESP32 pinout (NFC firmware)

| Signal | GPIO | Notes |
|--------|------|--------|
| PZEM RX / TX | **16** / **17** | UART2 — unchanged |
| PN532 SDA | **21** | I2C |
| PN532 SCL | **22** | I2C |
| PN532 IRQ | **4** | Optional |
| PN532 RST | **5** | Optional |
| Fotek SSR | **26** | Unchanged |

**Rev 1 pins 18/19** (125 kHz UART) — **unused** on Rev 2.

---

## Rev 2 — enrollment (site, once)

| Item | Qty |
|------|-----|
| PN532 module (desk) or USB PN532 | 1 |
| PC / phone enrollment tool | 1 (software TBD) |
| Blank MIFARE Classic 1K cards | Stock |

**Process:** Write `employee_id` + `display_name` → give card to worker. Worker change → **new card**, not platform edit.

See [`MACHINE_RUNTIME_NFC.md`](MACHINE_RUNTIME_NFC.md) for card layout.

---

## Rev 2 — where to buy (India)

| Part | Channel |
|------|---------|
| **PN532 MT0359 (frozen)** | **[Mifra Electronics](https://mifraelectronics.com/product/pn532-nfc-rfid-read-write-module/)** (Bangalore; +91 8073744810) |
| MIFARE Classic 1K cards | Mifra, Robu, Amazon — verify “13.56 MHz” / “MIFARE 1K” |
| ESP32, PZEM, SMPS, enclosures | Same as Rev 1 §12 |

---

## Revision history (continued)

| Rev | Date | Change |
|-----|------|--------|
| **2.0** | 2026-05-29 | **Frozen PN532** (Mifra MT0359) + **MIFARE Classic 1K**; id+name on card; I2C GPIO 21/22; supersedes 125 kHz worker RFID for new machines |
