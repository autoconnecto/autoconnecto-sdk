# Board verify (device path)

**Goal:** An external auditor finding a device/dashboard fault should be rare. Routine work must catch those faults before ship language is used.

Static checks (`tsc`, seed audits, reading code) are **not** enough for transport, control, OTA, RPC, or command-log changes.

## When this gate applies

Any change that touches:

- `sdk/` (examples, OTA, MQTT/HTTP transport, attribute/RPC callbacks)
- Backend paths that push to devices (MQTT shared attrs, RPC/commands, OTA chunk API, command log)
- Frontend widgets that **send** commands or shared attributes (Switch, Slider, RPC, GPIO, AttributeControl*, Command Log)

## Forbidden language until rows below are done

Do **not** say: fixed, verified, confirmed working, ready to retest as done, or ship-ready — for the items above — unless each relevant row is **Pass** or **Blocked (founder: no board)**.

Allowed when board unavailable: **Code review + static only — not board-verified.** Put that sentence in the reply and in CHANGELOG/Unreleased.

## Checklist (copy into the change reply)

Run against a real ESP32 (or HTTP sketch) on the tenant the founder uses. Prefer `AllFunctionTest_mqtt` or `WidgetCoverage_mqtt`.

| # | Check | Pass evidence |
|---|--------|----------------|
| B1 | **UI → API** | Named the exact HTTP route the widget calls (e.g. `POST /api/devices/:id/commands`). Logging/ack implemented on **that** route. |
| B2 | **Telemetry** | Serial shows publish; platform stores **all** keys in one interval (Free plan ≥ 30 s). |
| B3 | **Switch (shared attr)** | Toggle Switch → serial `[ATTR] channelN` → client attr confirms → widget sync. |
| B4 | **Switch (RPC)** | Widget `controlTransport=rpc` → serial `set_channel_state` / `relay_set` → reply. |
| B5 | **Command log** | After B4 (or RPC widget), row appears in Command Log for that device. |
| B6 | **OTA chunk** | Assign package → serial `chunk N ok` → `DOWNLOADED` → `VERIFIED` (or clear `FAILED`). No silent hang after `DOWNLOADING`. |
| B7 | **Secrets** | No live Wi‑Fi password or device token committed in examples. |

Mark **N/A** only if that surface was not in the change.

## Agent rules

1. Trace the **frontend API client** first; do not assume a parallel controller already logs or publishes.
2. Prefer one sketch + one dashboard path over “looks correct in source.”
3. If the founder’s board is offline, say so — do not invent Pass.
4. External audit findings that repeat a missed B-row are a **process failure**; fix the gate, not only the bug.

## Related

- Change hard gate: `CHANGE_MANAGEMENT.md` § Hard gates → Board / device path
- Cursor: `.cursor/rules/board-verify.mdc`
- OTA contract: `sdk/OTA.md`
