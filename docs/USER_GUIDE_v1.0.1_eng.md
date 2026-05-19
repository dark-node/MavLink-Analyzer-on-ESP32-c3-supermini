# MavLink Analyzer — User Guide v1.0.1

| | |
|---|---|
| Firmware | **1.0.1** (release) |
| Board | ESP32-C3 Super Mini |
| Web UI | http://192.168.4.1 |
| Wiring | [Technical spec §2](TECHNICAL_SPECIFICATION_v1.0_eng.md#2-hardware) |

---

## 1. Purpose

**MavLink Analyzer** sits between the **flight controller (FC)** and the **receiver** (or another UART MAVLink device):

1. **Transparent bridge** — all MAVLink frames pass between FC and receiver unchanged (arm, RC, telemetry).
2. **Browser log** — only the **receiver → FC** stream is recorded and displayed (commands, missions, RC link). **FC → receiver** telemetry goes through the bridge but **is not logged**.

---

## 2. Safety and limits

- ESP32-C3 UART lines are **3.3 V only**. Do not connect 5 V TTL without a level shifter.
- **Common GND** between FC, ESP, and receiver is mandatory.
- The Wi‑Fi access point has **no internet** — only your phone/laptop.
- USB/Monitor temperature is the **ESP die temperature**, not ambient air; 70–80 °C is possible with Wi‑Fi active.
- **Exit** turns off Wi‑Fi and enters **deep sleep** — a full **power cycle** is required for a new session.

---

## 3. Wiring (summary)

Details: **[technical specification §2](TECHNICAL_SPECIFICATION_v1.0_eng.md#2-hardware)**

| Connection | From | To ESP |
|------------|------|--------|
| FC → ESP (RX) | FC **TX** | **GPIO 20** (UART0 RX) |
| ESP → FC (TX) | FC **RX** | **GPIO 21** (UART0 TX) |
| Receiver → ESP | Receiver **TX** | **GPIO 4** (UART1 RX) |
| ESP → Receiver | Receiver **RX** | **GPIO 5** (UART1 TX) |
| Ground | All GND | **GND** on ESP |

UART: **8N1**, default **115200** baud (must match FC and receiver).

---

## 4. First power-on

### 4.1. Power and USB

1. Wire according to the table above.
2. Power the ESP (USB-C or your 3.3 V supply).
3. Optional: USB to PC, Serial Monitor **115200** — expect `BOOT OK`.

### 4.2. Wi‑Fi

| Parameter | Default |
|-----------|---------|
| SSID | `MavLink-Analyzer` |
| Password | `12345678` |
| Analyzer IP | `192.168.4.1` |

1. Connect your phone or laptop to `MavLink-Analyzer`.
2. Open in browser: **http://192.168.4.1** (no captive portal — enter the URL manually).

### 4.3. First session

On power-on the device is in **RUN** — the bridge is already active.

1. Confirm WebSocket status shows the live stream is connected.
2. **MAVLink log** should show **RCVR→FC** lines when the receiver sends data.
3. Verify arm, RC, and telemetry on FC/receiver as without the analyzer inline.

---

## 5. Web interface

### 5.1. Header

- **RUN** / **STOP** state pill.
- **EN / UA** — UI language (stored in browser).
- **Debug** — optional technical blocks (memory, raw JSON). Can stay off for normal use.

### 5.2. Control buttons

| Button | Action |
|--------|--------|
| **STOP** | Stop bridge and UART; freeze log; edit P1–P5, P7–P8 |
| **START** | Apply form settings and restart bridge |
| **Save log** | Download `MavLinkLog_N.txt` (up to 100 frames on ESP) |
| **Browser log** | Shown when P6 is on — download full browser log |
| **Exit** | End session: Wi‑Fi off, deep sleep; power cycle to restart |

### 5.3. Live stats

Updated every **5 seconds**:

| Metric | Meaning |
|--------|---------|
| Log /s | Valid RCVR→FC log frames per second |
| Telem /s | Valid FC→receiver frames (counter only, not logged) |
| Bridge B/s | Bytes through bridge (both directions) |
| CRC /s | CRC errors on receiver→FC line |

### 5.4. Settings P1–P8

| ID | Name | Description |
|----|------|-------------|
| P1 | MAVLink | `auto`, `v1`, or `v2` for log filtering |
| P2 | Baudrate | 9600 … 921600; same on both UARTs |
| P3 | Parsing | **Off** — hex; **On** — structured decode |
| P4 | Filter HB | **On** — HEARTBEAT (msgid 0) excluded from log only |
| P5 | Passthrough | **On** — bridge on; **Off** — no UART forwarding |
| P6 | Browser log | Store each log line in browser localStorage |
| P7 | Wi‑Fi SSID | 1–32 characters |
| P8 | Wi‑Fi password | 8–63 characters |

**Save** — write to NVS. **Reboot** — restart (for Wi‑Fi after save). **Reset** — factory defaults.

### 5.5. Parsing On — decoded messages

| msgid | Name |
|-------|--------|
| 0 | HEARTBEAT |
| 24 | GPS_RAW_INT |
| 30 | ATTITUDE |
| 33 | GLOBAL_POSITION_INT |
| 74 | VFR_HUD |
| 75 | COMMAND_INT |
| 76 | COMMAND_LONG |
| 152 | MEMINFO (ArduPilot) |
| 163 | AHRS (ArduPilot) |
| 253 | STATUSTEXT |

Other valid RCVR→FC frames appear as **raw** hex.

### 5.6. Debug (optional)

When **Debug** is on: memory/flash block and raw **GET /api/status** JSON.

**Release** firmware v1.0.1 uses **real UART only** (no packet emulation).

---

## 6. Typical workflows

### 6.1. Change baud rate

1. **STOP** → set **Baudrate** → **Save** → **START**

### 6.2. Change Wi‑Fi

1. **STOP** → new **SSID** / **password** → **Save** → **Reboot** → reconnect Wi‑Fi

### 6.3. Save flight log

**Save log** → `MavLinkLog_0.txt`, `MavLinkLog_1.txt`, … (counter resets after full power off)

### 6.4. Full browser log (P6)

Enable **Browser log** → after session **Browser log** → `MavLinkBrowserLog.txt`

### 6.5. Shutdown

**Exit** → confirm → power off; power on again for a new session

---

## 7. What is normal

| Observation | Explanation |
|---------------|-------------|
| No FC ATTITUDE/GPS in log | **Normal** — log is RCVR→FC only |
| Telem /s > 0, empty FC-side log | **Normal** |
| No HEARTBEAT in log | **Normal** with Filter HB on |
| Occasional CRC /s | Check wiring and baud |
| No Wi‑Fi after Exit | **Normal** — power cycle required |

---

## 8. Firmware update

```bash
pio run -e release -t upload
pio device monitor
```

Default PlatformIO environment: **release**.

---

## 9. Related documents

| File | Content |
|------|---------|
| [TECHNICAL_SPECIFICATION_v1.0_eng.md](TECHNICAL_SPECIFICATION_v1.0_eng.md) | Technical specification (wiring — §2) |
| [USER_GUIDE_v1.0.1_ua.md](USER_GUIDE_v1.0.1_ua.md) | User guide (Ukrainian) |
| [PUBLIC_CODE_MANIFEST.md](PUBLIC_CODE_MANIFEST.md) | What is published on GitHub |

---

*MavLink Analyzer v1.0.1 — release*
