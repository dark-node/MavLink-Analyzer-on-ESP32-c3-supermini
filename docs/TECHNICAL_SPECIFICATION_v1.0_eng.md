# Technical Specification — Implementation v1.0.1 (release)

## MavLink Analyzer — Firmware for ESP32-C3 Super Mini

| Field | Value |
|-------|--------|
| Document version | 1.0.1 |
| Firmware (release) | **1.0.1** |
| Date | 2026-05-16 |
| Status | **Implemented (release)** |
| Target hardware | ESP32-C3 Super Mini |
| Default build | PlatformIO `env:release` |
| Ukrainian version | [TECHNICAL_SPECIFICATION_v1.0_ua.md](TECHNICAL_SPECIFICATION_v1.0_ua.md) |
| User guide | [USER_GUIDE_v1.0.1_eng.md](USER_GUIDE_v1.0.1_eng.md) |
| Wiring | [§2 Hardware](#2-hardware) |

---

## Table of contents

1. [Purpose](#1-purpose)
2. [Hardware](#2-hardware)
3. [Software architecture](#3-software-architecture)
4. [Modes and states](#4-modes-and-states)
5. [Wi-Fi](#5-wi-fi)
6. [Settings and NVS](#6-settings-and-nvs)
7. [MAVLink: bridge, CRC, log, parsing](#7-mavlink-bridge-crc-log-parsing)
8. [Web interface and API](#8-web-interface-and-api)
9. [USB Serial](#9-usb-serial)
10. [Temperature and shutdown](#10-temperature-and-shutdown)
11. [Build and resources](#11-build-and-resources)
12. [Implemented capabilities v1.0.1](#12-implemented-capabilities-v101)

---

## 1. Purpose

**MavLink Analyzer** is an ESP32-C3 Super Mini module between the **flight controller (FC)** and **receiver** (UART MAVLink).

### 1.1 Release v1.0.1 functions

1. **Transparent bidirectional MAVLink bridge** UART0 ↔ UART1: frame bytes forwarded unchanged.
2. **Receiver → FC stream analysis** (UART1 RX): frame sync, CRC check, ring log buffer, browser display via WebSocket.
3. **FC → receiver telemetry counter** (UART0 RX): valid frames in `telemetry_frames_per_s` **without logging**.
4. **Web UI** in SoftAP mode: settings, RUN/STOP control, log export, statistics, bilingual UI (EN/UA).
5. **Settings persistence** in NVS across power cycles.

### 1.2 Operating principles

- Bridge and log are **separated**: HEARTBEAT filter affects log only, not forwarding.
- Log contains only **RCVR→FC** frames with direction in the text line.
- Release firmware uses **real UART only**; packet emulation code is not included in release builds.
- Wi-Fi AP is isolated: one client, no internet access.

---

## 2. Hardware

### 2.1 Board

| Parameter | Value |
|-----------|--------|
| Model | ESP32-C3 Super Mini |
| UART logic | 3.3 V |
| USB | USB-C CDC (flash, service port 115200) |
| PlatformIO board | `esp32-c3-devkitm-1` |

### 2.2 UART pinout

| Interface | Signal | GPIO | Connection |
|-----------|--------|------|------------|
| UART0 | RX | **20** | From **FC TX** |
| UART0 | TX | **21** | To **FC RX** |
| UART1 | RX | **4** | From **receiver TX** |
| UART1 | TX | **5** | To **receiver RX** |

Format: **8N1**, baud from NVS (same on UART0 and UART1).

### 2.3 Wiring diagram

```
  Flight Controller              MavLink Analyzer              Receiver
        TX ──────────────────────► GPIO20 (UART0 RX)
        RX ◄────────────────────── GPIO21 (UART0 TX)
       GND ─────────────────────── GND ─────────────────────── GND
                                        │
        TX ◄────────────────────── GPIO5  (UART1 TX)
        RX ──────────────────────► GPIO4  (UART1 RX)
```

Wiring details are in §2.2–2.3 above.

### 2.4 Traffic directions

| Direction | Path | Bridge | Log / parse |
|-----------|------|--------|-------------|
| **FC → receiver** | FC TX → GPIO20 → bridge → GPIO5 → receiver RX | Yes | No (stats only) |
| **Receiver → FC** | Receiver TX → GPIO4 → bridge → GPIO21 → FC RX | Yes | Yes (tap on UART1 RX) |

---

## 3. Software architecture

### 3.1 Stack

| Component | Implementation |
|-----------|----------------|
| Build | PlatformIO, Arduino (ESP32) |
| NVS | `Preferences`, namespace `mavlink` |
| Wi-Fi | SoftAP |
| HTTP / WebSocket | ESPAsyncWebServer-esphome, AsyncTCP-esphome |
| MAVLink dialect | `lib/mavlink/analyzer.xml` (in public repo); generated C headers — not published |
| Wire CRC | **295** msgid table (common + ardupilotmega) — not published |

### 3.2 Firmware modules

| Module | Files | Responsibility |
|--------|-------|----------------|
| `config` | `include/config.h` | GPIO, buffer limits, thermal, version |
| `settings` | `settings.cpp/h` | P1–P8, NVS, pending/active, JSON API |
| `app_state` | `app_state.cpp/h` | RUN / STOP / EXIT |
| `bridge` | `bridge.cpp/h` | UART0 ↔ UART1, passthrough |
| `mavlink_tap` | `mavlink_tap.cpp/h` | Parser on UART1 RX; telemetry on UART0 RX |
| `mavlink_crc` | `mavlink_crc.cpp/h` | CRC validate/finalize |
| `mavlink_format` | `mavlink_format.cpp/h` | Raw / parsed log lines |
| `log_ring` | `log_ring.cpp/h` | 100 entries, up to 768 chars each |
| `stats` | `stats.cpp/h` | fps, B/s, CRC/s (1 s window) |
| `wifi_ap` | `wifi_ap.cpp/h` | SoftAP, 1 client |
| `web_server` | `web_server.cpp/h`, `web_page.h` | UI, REST, WebSocket |
| `sys_info` | `sys_info.cpp/h` | RAM/flash/NVS for `/api/status` |
| `thermal` | `thermal.cpp/h` | Die sensor, WARN / shutdown |
| `shutdown` | `shutdown.cpp/h` | Deep sleep after Exit / thermal |
| `debug_serial` | `debug_serial.cpp/h` | USB BOOT OK / ERR_* |

### 3.3 Data flow

```
UART0 ◄──► [bridge] ◄──► UART1
     │                         │
     │ FC→RCVR (telem stats)   │ RCVR→FC
     │                         ▼
     │                  [mavlink_tap UART1 RX]
     │                         ▼
     │              CRC (295 msgid table)
     │                         ▼
     │              HEARTBEAT filter (log only)
     │                         ▼
     │              [mavlink_format]
     │                         ▼
     │              [log_ring] ──► WebSocket ──► browser
```

---

## 4. Modes and states

### 4.1 State machine

| State | After power-on | Bridge | UART | Log | Edit P1–P5, P7–P8 |
|-------|----------------|--------|------|-----|-------------------|
| **RUN** | Yes (default) | On (if P5) | Active | Live | Locked |
| **STOP** | After STOP | Off | Idle | Frozen | Allowed |
| **EXIT** | After Exit | Off | Off | — | — → deep sleep |

After **EXIT** or thermal shutdown — **power cycle** required for a new session.

### 4.2 Web transitions

- **START**: `applyPending()`, `bridge::start(baud)`, WebSocket `state: RUN`.
- **STOP**: stop bridge, `state: STOP`.
- **Exit**: deep sleep, WebSocket notify, Wi-Fi off.

### 4.3 Reboot

`POST /api/reboot` → `esp_restart()`. NVS SSID/password and settings apply after previous **Save**.

---

## 5. Wi-Fi

| Parameter | Value |
|-----------|--------|
| Mode | SoftAP |
| Default SSID / password | `MavLink-Analyzer` / `12345678` |
| AP IP | `192.168.4.1` |
| Channel | 6 |
| Max clients | **1** |
| Security | WPA2-PSK |

Change P7–P8: **Save** → **Reboot**.

---

## 6. Settings and NVS

### 6.1 Parameters P1–P8

| ID | NVS key | Type | Default | Applied |
|----|---------|------|---------|---------|
| P1 | `proto` | enum | `auto` | On **START** |
| P2 | `baud` | uint32 | 115200 | On **START** |
| P3 | `parse` | bool | Off | On **START** |
| P4 | `filt_hb` | bool | On | On **START** |
| P5 | `pass` | bool | On | On **START** |
| P6 | — | — | Off | Browser only |
| P7 | `ssid` | string | `MavLink-Analyzer` | After **Reboot** |
| P8 | `wpass` | string | `12345678` | After **Reboot** |

NVS `testpkt` exists for **dev** builds (emulation); unused in **release**.

### 6.2 Settings API

- `GET /api/settings` — state, editable, active, pending, nvs_ok, test_packets_supported.
- `POST /api/settings` — pending JSON (**STOP** only).
- `POST /api/save`, `POST /api/reset`.

---

## 7. MAVLink: bridge, CRC, log, parsing

### 7.1 Bridge

- Bidirectional forward, read chunk up to 256 bytes per `poll()`.
- UART RX buffer 1024 bytes per port.
- **Passthrough Off**: no forwarding; log idle.
- Main loop calls `bridge::poll()` ~every 1 ms in RUN.

### 7.2 CRC validation

- Table of **295** entries: MAVLink **common** + **ardupilotmega** (implementation and table not in the public repository).
- Fallback lookup in **analyzer** dialect.
- Unknown msgid: frame **rejected** for log.
- **MAVLink v1** (`0xFE`) and **v2** (`0xFD`) supported.

### 7.3 mavlink_tap

- Separate parsers: **UART1 RX** (log), **UART0 RX** (telemetry count).
- On valid CRC: P1 version filter, P4 HEARTBEAT filter, format, push to log_ring, WebSocket line.
- Counters: accepted, crc_error, filtered.

### 7.4 Log buffer

| Parameter | Value |
|-----------|--------|
| Capacity | 100 MAVLink frames |
| Max text per frame | **768** chars |
| Type | Ring buffer |
| Overflow | Drop oldest |

### 7.5 Parsing Off (raw)

Line includes: uptime ms, MAV v1/v2, **RCVR→FC**, sysid, compid, msgid, seq, len, **full payload hex** (within 768 char entry limit).

### 7.6 Parsing On (parsed)

Structured output for msgids in `analyzer.xml`:

| msgid | Message |
|-------|---------|
| 0 | HEARTBEAT |
| 24 | GPS_RAW_INT |
| 30 | ATTITUDE |
| 33 | GLOBAL_POSITION_INT |
| 74 | VFR_HUD |
| 75 | COMMAND_INT |
| 76 | COMMAND_LONG |
| 152 | MEMINFO |
| 163 | AHRS |
| 253 | STATUSTEXT |

Other valid CRC-correct frames → **raw** as §7.5.

### 7.7 HEARTBEAT filter

- **On**: msgid 0 not logged; bridge still forwards.
- **Off**: HEARTBEAT in log.

### 7.8 Statistics

| Metric | Description |
|--------|-------------|
| `uptime_ms` | Since boot |
| `log_frames_per_s` | Valid log frames/s |
| `telemetry_frames_per_s` | FC→receiver frames/s |
| `bridge_bytes_per_s` | Bridge bytes/s |
| `crc_errors_per_s` | UART1 RX CRC errors/s |
| Totals | Session accumulators |
| `mavlink.*` | Accepted, CRC errors, log count |
| `bridge_*` | Active flag and byte counters |

UI refresh: 5 s poll + WebSocket `state` events.

---

## 8. Web interface and API

### 8.1 Page `/` (`web_page.h` in PROGMEM)

- Responsive layout, dark theme.
- Languages **EN / UA** (`localStorage`).
- **Debug** mode: memory block, raw JSON status.

Sections: controls, live stats, settings P1–P8, debug tools, MAVLink log.

### 8.2 WebSocket `/ws`

| JSON type | Content |
|-----------|---------|
| `banner` | Session line (version, baud, MAVLink, parsing, …) |
| `history` | Up to 30 recent lines in one message |
| `line` | New log line |
| `state` | RUN / STOP |
| `exit` | Session ended message |

On connect: banner + history + state. Queue-full clients skipped.

### 8.3 REST API

| Method | Path | Description |
|--------|------|-------------|
| GET | `/` | HTML UI |
| GET | `/api/status` | Full status JSON + storage |
| GET | `/api/log` | Buffer dump |
| POST | `/api/start`, `/api/stop` | State |
| GET/POST | `/api/settings` | Read / write pending |
| POST | `/api/save`, `/api/reset`, `/api/reboot`, `/api/exit` | Actions |
| GET | `/api/save-log` | `MavLinkLog_N.txt` download |
| POST | `/api/debug` | Dev build: test_packets |

### 8.4 Save Log

`MavLinkLog_0.txt`, … — counter resets on power-on, UTF-8.

### 8.5 Browser log (P6)

localStorage keys; download `MavLinkBrowserLog.txt`; quota handling in UI.

---

## 9. USB Serial

| Event | Line |
|-------|------|
| Success | `BOOT OK` |
| NVS warning | `ERR_NVS` then `BOOT OK` |
| Init failure | `ERR_WIFI`, `ERR_WEB`, `ERR_UART` |

115200 baud. No MAVLink on USB.

---

## 10. Temperature and shutdown

| Threshold | Action |
|-----------|--------|
| ≥ **80 °C** die | WARN in UI, latch, clear below **72 °C** |
| ≥ **85 °C** | Emergency deep sleep |
| Poll | Every **10 s** |

---

## 11. Build and resources

### 11.1 PlatformIO

| Environment | Use | `BRIDGE_SIMULATION` | Version |
|-------------|-----|---------------------|---------|
| **release** (default) | Field / production | 0 | 1.0.1 |
| **dev** | Desk test, Debug test packets | 1 | 1.0.1 |

```bash
pio run -e release -t upload
```

### 11.2 Release resources (typical)

| Resource | Usage |
|----------|--------|
| Flash | ~884 KB (~67.5 %) |
| RAM static | ~125 KB (~38 %) |
| Free flash | ~426 KB for OTA headroom |

---

## 12. Implemented capabilities v1.0.1

- Bidirectional UART bridge 8N1, baud 9600–921600 from NVS.
- Log tap **RCVR→FC only**; FC→receiver in telemetry stats.
- CRC table **295** msgids (common + ArduPilotMega).
- Ring log **100×768** chars; WebSocket streaming.
- Raw and parsed (10 message types); raw fallback.
- HEARTBEAT log filter.
- NVS P1–P8, Save, Reset, Reboot.
- SoftAP, 1 client, http://192.168.4.1.
- RUN/STOP/EXIT, deep sleep, thermal protection.
- Save Log / browser log (P6).
- UI EN/UA, debug memory/status views.
- `/api/status` with heap, flash, NVS storage.
- Release: no injected packets on FC↔receiver link.

---

## Document history

| Version | Date | Changes |
|---------|------|---------|
| 1.0.1 | 2026-05-16 | Implemented release v1.0.1 specification |

---

*End of document*
