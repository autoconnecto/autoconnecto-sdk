# Machine runtime — Rev 3 worker BLE (Android)

**Status:** Pilot  
**Firmware:** [`tools/machine-runtime-ble/`](../tools/machine-runtime-ble/)  
**Worker app:** [`tools/machine-worker-app/`](../tools/machine-worker-app/) — **BLE only** (no HTTP/Cognito on phone)

## Identity

| Field | Where set | ESP uses |
|-------|-----------|----------|
| `machine_code` | Fleet widget **Setup** → SHARED | Dashboard / reports |
| `machine_slot` | Fleet widget **Setup** → SHARED | BLE advert `AC-007`, NVS |

Physical label on machine should match BLE name (**AC-001**, etc.).

## Worker app (BLE-only)

1. Worker enters **ID + name** once (stored on phone until **Edit profile**).
2. Scans and **selects** one machine (`AC-###`) — saved until **Change machine**.
3. App **auto-reconnects** to the saved machine if BLE drops (no rescan).
4. **End shift** stops the session only; machine assignment is kept.
5. **START** sends `operator_id` / `operator_name` over BLE → ESP → MQTT.

No HTTP or Cognito on the worker phone. **Tenant scope stays on the platform** (logged-in dashboard user → device in that tenant). The ESP is provisioned with a **device token** for one tenant; MQTT SHARED attrs (tool life, allow-run, slot) are per device. The worker app only reads what the ESP exposes over BLE.

## Multi-tenant tool life (no env on phone)

```
Dashboard (tenant from login JWT)
  → Machine Fleet Setup widget saves tool life for deviceId
  → SERVER + SHARED device_attributes (that tenant only)
  → MQTT retained SHARED → ESP (device token)
  → BLE status: tool_remaining, allow_run
  → Worker app displays "Tool life left"
```

On BLE connect/reconnect the app sends `{"cmd":"sync_attrs"}` so the ESP pulls the latest SHARED snapshot from MQTT (e.g. after admin changed limit in Setup).

## BLE GATT

| UUID | Role |
|------|------|
| Service `a7c50001-0001-4000-8000-ac0000010001` | Autoconnecto worker |
| Char `a7c50002-...0002` | Write JSON commands |
| Char `a7c50003-...0003` | Read/notify status JSON |

### Commands (write cmd char)

```json
{"cmd":"start","operator_id":"W1","operator_name":"Rajesh Kumar"}
{"cmd":"stop"}
{"cmd":"job_add"}
{"cmd":"job_remove"}
{"cmd":"heartbeat"}
{"cmd":"sync_attrs"}
```

- **start** — rejected if another `operator_id` already has an active session; same operator **resumes** (job count kept).
- **stop** — session off, SSR OFF, **session job count resets to 0**.
- **heartbeat** — while session on and BLE connected; timeout **15 min** without heartbeat ends session (jobs reset).
- **sync_attrs** — ESP requests fresh MQTT SHARED attributes (tool life, allow-run); worker app sends on connect/reconnect.

### Status notify

```json
{
  "slot": 7,
  "session": true,
  "jobs": 12,
  "allow_run": true,
  "tool_life_enabled": true,
  "tool_remaining": 8,
  "tool_limit": 500,
  "operator_id": "W1",
  "operator_name": "Rajesh Kumar",
  "ble_linked": true,
  "session_busy": false
}
```

`tool_remaining` / `tool_limit` are present when tool life is enabled on the platform (SHARED attrs synced to ESP). Omitted or `tool_life_enabled: false` when tool counter is off.

`jobs` = **this session only** (operator +/−). Resets on **stop**, heartbeat timeout, or tool-life block. **Not** cumulative across shifts.

Tool wear uses platform `machine_tool_cycles_used` (cumulative). Dashboard productivity charts use PZEM current patterns — separate from the worker button count.

`session_busy: true` when a different operator tried **start**.

## Session / SSR

**Policy (industrial):** SSR (machine enable) = **session active AND allow_run**.

| Action | Session | SSR |
|--------|---------|-----|
| App **START SESSION** | ON | ON (if allow_run) |
| App **End shift** / `stop` | OFF | OFF |
| **Tool life exhausted** | **Stays ON** (worker still at press) | **OFF** |
| Admin **Reset tool** in Setup | Unchanged | **ON** again (if session still ON) |
| Platform `machine_allow_run=false` (admin) | Unchanged | OFF |
| BLE disconnect | No change | No change |

- **start** → session on, SSR ON when `allow_run`
- **stop** / End shift → session off, SSR OFF, jobs reset to **0**
- **Tool life hit** → `allow_run=false`, SSR OFF, session and job count **kept** until End shift. Platform SERVER/SHARED updated from device CLIENT mirror (prevents stale `allow_run=true` re-enabling the press on `sync_attrs`).
- **Reset tool** (dashboard) → MQTT `machine_allow_run=true` + new remaining → SSR ON if session still active

## Power cycle survival (Rev 3.1+)

Industrial machines must **stay ON** across ESP power loss while a worker session is active.

**On every state change** the firmware writes a full snapshot to **NVS** (`ac_mach`):

- `session_active`, `operator_id`, `operator_name`
- `machine_cycle_count`, `session_start_ts`, `session_end_ts`
- `machine_allow_run`, `machine_slot`, tool-life cache

**Boot sequence:**

1. **Load NVS** → restore session + job count → **SSR ON** only if `session_active && allow_run`
2. **SDK + BLE + MQTT init** (no HTTP during `setup()` — avoids crash / watchdog reset)
3. **MQTT connect**: shared attrs via MQTT (primary). HTTP fallback only if MQTT snapshot missing after ~5s.
4. **BLE/SSR side effects run only in `loop()`** — never from the MQTT callback thread (prevents crash/reboot loop).

Serial confirmation after power cycle:

```
[NV] restore v=2 slot=1 session=1 jobs=5 allow_run=1 op=W1
[SSR] boot output=ON
```

Worker app does **not** need to tap START again — BLE reconnects to the restored session; job count is preserved.

**Authority:** `machine_allow_run` (SHARED) still wins — if platform sets it `false`, session ends even after restore.

## Telemetry (ESP → platform)

`machine_current_a`, `machine_voltage_v`, `machine_power_w`, `machine_operator_id`, `machine_operator_name`, `machine_session_active`, `machine_session_start_ts`, `machine_session_end_ts`, `machine_cycle_count`

| Key | Description |
|-----|-------------|
| `machine_session_start_ts` | Unix **seconds** (NTP) when worker tapped **START SESSION** |
| `machine_session_end_ts` | Unix seconds when session ended (**End shift**, stop, heartbeat timeout, tool block); `0` while session active |

Mirrored to **CLIENT** attributes on start/end and every 30 s.

## Field reliability (BLE advertising)

When the phone walks away, Android often drops BLE **without** a clean disconnect. On ESP32 this is a known NimBLE issue: `onDisconnect` may not fire, the firmware can think a peer is still connected, and **advertising stops** — the phone scan finds nothing until the ESP is power-cycled.

**Firmware mitigation** (`Machine_Runtime_BLE_mqtt.ino`):

- Every **5 s**, reconcile using `bleServer->getConnectedCount()` (not a boolean flag).
- If **0 peers** → restart advertising automatically.
- If a peer is connected but **no GATT activity for 45 s** → drop stale link and restart advertising.

**App mitigation:** wait **1.5 s** after disconnect before reconnecting (reduces NimBLE ghost-connection races).

After reflash, serial log should show `[BLE] advertising restart (no_peers)` or `[BLE] ghost link cleared` within ~5 s of the phone leaving — **no power cycle**.

## Optional external BLE coprocessor (production / 250 machines)

On-board ESP32 BLE + WiFi + MQTT on one radio is fragile. For industrial reliability, split radios:

| Module | Role | Interface to ESP32 | Notes |
|--------|------|-------------------|--------|
| **Nordic nRF52840** (Raytac MDBT50Q, Ebyte E73-2G4M08S1C) | BLE only | **UART** (3.3 V, 115200) Nordic UART Service or custom GATT bridge | Best stability; keep same JSON `cmd` contract over UART |
| **HM-19 / JDY-10** (CC2541) | BLE UART | UART AT or transparent serial | Cheap pilot only; weak for 10+ concurrent ops |
| **Second ESP32-C3** | BLE peripheral only | UART (`RX`/`TX`) | Reuse NimBLE sketch on C3; main ESP32 = WiFi/MQTT/PZEM only |

Wiring (nRF52840 UART example): ESP32 `GPIO17 TX` → nRF `RX`, ESP32 `GPIO16 RX` ← nRF `TX`, common GND, nRF `3V3` from ESP 3.3 V (≥200 mA). No change to worker app if nRF exposes the same GATT UUIDs (`a7c50001-…`) or a UART bridge translates JSON.

**Does not fix SSR/session logic** — only reduces reboot/BLE-advert drops from WiFi/BLE coexistence.

## Owner setup

1. Fleet widget **Setup** → add machine, slot, code, thresholds  
2. Flash `Machine_Runtime_BLE_mqtt.ino`  
3. Worker app → pin machine → START  

E2E: [`tools/machine-worker-app/E2E_TEST.md`](../tools/machine-worker-app/E2E_TEST.md)
