# Public code manifest

This repository is **not** a complete firmware tree. It is published for
documentation and portfolio review only.

## Published (full)

| Path | Notes |
|------|--------|
| `docs/USER_GUIDE_v1.0.1_ua.md` | User guide (Ukrainian) |
| `docs/USER_GUIDE_v1.0.1_eng.md` | User guide (English) |
| `docs/TECHNICAL_SPECIFICATION_v1.0_ua.md` | Technical specification (Ukrainian) |
| `docs/TECHNICAL_SPECIFICATION_v1.0_eng.md` | Technical specification (English) |
| `screenshots/` | Web UI screenshots (PC + mobile) |
| `include/*.h` | Module API headers |
| `src/app_state.cpp` | Global app state |
| `src/log_ring.cpp` | Log ring buffer |
| `src/debug_serial.cpp` | USB debug serial helpers |
| `lib/mavlink/analyzer.xml` | Parsed MAVLink message list (no generated C) |
| `lib/mavlink/README.md` | Dialect overview |
| `platformio.ini` | Board/deps reference (tree does not build) |
| `LICENSE`, `NOTICE`, `REPOSITORY_POLICY.md`, `CONTACT.md` | Legal / policy |

## Published (stub only)

These files exist so visitors see module boundaries. They contain **no**
implementation.

| File | Module |
|------|--------|
| `src/main.cpp` | Application entry |
| `src/bridge.cpp` | UART bridge |
| `src/mavlink_tap.cpp` | MAVLink tap / frame sync |
| `src/mavlink_crc.cpp` | CRC validation |
| `src/mavlink_format.cpp` | Parsed / raw log formatting |
| `src/settings.cpp` | NVS settings |
| `src/wifi_ap.cpp` | Wi-Fi SoftAP |
| `src/web_server.cpp` | HTTP / WebSocket |
| `src/web_page.h` | Web UI (HTML/CSS/JS) |
| `src/stats.cpp` | Statistics |
| `src/thermal.cpp` | Temperature monitor |
| `src/shutdown.cpp` | Deep sleep / exit |
| `src/sys_info.cpp` | RAM / flash info |

## Not published

| Item | Reason |
|------|--------|
| `lib/mavlink/generated/` | Generated MAVLink C headers |
| `lib/mavlink/crc_common_table.h` | CRC lookup table (295 msgids) |
| `tools/` | mavgen, CRC generator, export scripts |
| Full `src/*.cpp` for core modules | Proprietary implementation |
| Firmware `.bin` | Usage restricted by LICENSE |

## Build status

**You cannot build working firmware from this repository.** Stubs and missing
generated assets prevent a complete link. See [LICENSE](../LICENSE).
