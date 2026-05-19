# Технічна специфікація — реалізація v1.0.1 (release)

## MavLink Analyzer — прошивка для ESP32-C3 Super Mini

| Поле | Значення |
|------|----------|
| Версія документа | 1.0.1 |
| Версія прошивки (release) | **1.0.1** |
| Дата | 2026-05-16 |
| Статус | **Реалізовано (release)** |
| Цільове залізо | ESP32-C3 Super Mini |
| Збірка за замовчуванням | PlatformIO `env:release` |
| Англомовна версія | [TECHNICAL_SPECIFICATION_v1.0_eng.md](TECHNICAL_SPECIFICATION_v1.0_eng.md) |
| Інструкція користувача | [USER_GUIDE_v1.0.1_ua.md](USER_GUIDE_v1.0.1_ua.md) |
| Підключення дротів | [§2 Апаратна частина](#2-апаратна-частина) |

---

## Зміст

1. [Призначення](#1-призначення)
2. [Апаратна частина](#2-апаратна-частина)
3. [Архітектура ПЗ](#3-архітектура-пз)
4. [Режими та стани](#4-режими-та-стани)
5. [Wi-Fi](#5-wi-fi)
6. [Налаштування та NVS](#6-налаштування-та-nvs)
7. [MAVLink: міст, CRC, лог, парсинг](#7-mavlink-міст-crc-лог-парсинг)
8. [Веб-інтерфейс та API](#8-веб-інтерфейс-та-api)
9. [USB Serial](#9-usb-serial)
10. [Температура та завершення роботи](#10-температура-та-завершення-роботи)
11. [Збірка та ресурси](#11-збірка-та-ресурси)
12. [Реалізовані можливості v1.0.1](#12-реалізовані-можливості-v101)

---

## 1. Призначення

**MavLink Analyzer** — автономний модуль на ESP32-C3 Super Mini між **політним контролером (FC)** та **приймачем** (UART MAVLink).

### 1.1. Функції release v1.0.1

1. **Прозорий двосторонній MAVLink-міст** UART0 ↔ UART1: байти передаються без зміни вмісту кадрів.
2. **Аналізатор потоку приймач → FC** (UART1 RX): синхронізація кадрів, перевірка CRC, кільцевий буфер логу, відображення в браузері через WebSocket.
3. **Лічильник телеметрії FC → приймач** (UART0 RX): валідні кадри враховуються в статистиці `telemetry_frames_per_s`, **без запису в лог**.
4. **Веб-інтерфейс** у режимі SoftAP: налаштування, керування RUN/STOP, збереження логу, статистика, двомовний UI (EN/UA).
5. **Збереження налаштувань** у NVS між циклами живлення.

### 1.2. Принципи роботи

- Міст і лог **розділені**: фільтр HEARTBEAT впливає лише на лог, не на пересилку.
- У лог потрапляють лише кадри напрямку **RCVR→FC** з міткою в тексті рядка.
- Release-прошивка працює **лише з реальним UART**; код емуляції пакетів у збірку release не входить.
- Wi-Fi AP ізольований: один клієнт, без виходу в інтернет.

---

## 2. Апаратна частина

### 2.1. Плата

| Параметр | Значення |
|----------|----------|
| Модель | ESP32-C3 Super Mini |
| Логіка UART | 3.3 V |
| USB | USB-C, CDC (прошивка, service-порт 115200) |
| PlatformIO board | `esp32-c3-devkitm-1` |

### 2.2. Розпіновка UART

| Інтерфейс | Сигнал | GPIO | Призначення |
|-----------|--------|------|-------------|
| UART0 | RX | **20** | Прийом від **TX FC** |
| UART0 | TX | **21** | Передача на **RX FC** |
| UART1 | RX | **4** | Прийом від **TX приймача** |
| UART1 | TX | **5** | Передача на **RX приймача** |

Параметри: **8N1**, baudrate з NVS (однаковий на UART0 і UART1).

### 2.3. Схема підключення

```
  Flight Controller              MavLink Analyzer              Receiver
        TX ──────────────────────► GPIO20 (UART0 RX)
        RX ◄────────────────────── GPIO21 (UART0 TX)
       GND ─────────────────────── GND ─────────────────────── GND
                                        │
        TX ◄────────────────────── GPIO5  (UART1 TX)
        RX ──────────────────────► GPIO4  (UART1 RX)
```

Деталі підключення — у §2.2–2.3 вище.

### 2.4. Напрямки трафіку

| Напрямок | Шлях | Міст | Лог / парсинг |
|----------|------|------|----------------|
| **FC → приймач** | FC TX → GPIO20 → bridge → GPIO5 → RX приймача | Так | Ні (лише stats) |
| **Приймач → FC** | TX приймача → GPIO4 → bridge → GPIO21 → FC RX | Так | Так (tap на UART1 RX) |

---

## 3. Архітектура ПЗ

### 3.1. Стек

| Компонент | Реалізація |
|-----------|------------|
| Збірка | PlatformIO, Arduino framework (ESP32) |
| NVS | `Preferences`, namespace `mavlink` |
| Wi-Fi | SoftAP |
| HTTP / WebSocket | ESPAsyncWebServer-esphome, AsyncTCP-esphome |
| MAVLink dialect | `lib/mavlink/analyzer.xml` (у публічному репо); згенеровані C-заголовки — не опубліковані |
| CRC на лінії | Таблиця **295** msgid (common + ardupilotmega) — не опублікована |

### 3.2. Модулі прошивки

| Модуль | Файли | Відповідальність |
|--------|-------|------------------|
| `config` | `include/config.h` | GPIO, ліміти буферів, thermal, версія |
| `settings` | `settings.cpp/h` | P1–P8, NVS, pending/active, JSON API |
| `app_state` | `app_state.cpp/h` | RUN / STOP / EXIT |
| `bridge` | `bridge.cpp/h` | UART0 ↔ UART1, passthrough |
| `mavlink_tap` | `mavlink_tap.cpp/h` | Парсер кадрів на UART1 RX; телеметрія UART0 RX |
| `mavlink_crc` | `mavlink_crc.cpp/h` | CRC validate/finalize |
| `mavlink_format` | `mavlink_format.cpp/h` | Raw / parsed рядки логу |
| `log_ring` | `log_ring.cpp/h` | 100 записів, до 768 символів кожен |
| `stats` | `stats.cpp/h` | fps, B/s, CRC/s (вікно 1 с) |
| `wifi_ap` | `wifi_ap.cpp/h` | SoftAP, 1 клієнт |
| `web_server` | `web_server.cpp/h`, `web_page.h` | UI, REST, WebSocket |
| `sys_info` | `sys_info.cpp/h` | RAM/flash/NVS для `/api/status` |
| `thermal` | `thermal.cpp/h` | Датчик кристала, WARN / shutdown |
| `shutdown` | `shutdown.cpp/h` | Deep sleep після Exit / thermal |
| `debug_serial` | `debug_serial.cpp/h` | USB BOOT OK / ERR_* |

### 3.3. Потік даних

```
UART0 ◄──► [bridge] ◄──► UART1
     │                         │
     │ FC→RCVR (telem stats)   │ RCVR→FC
     │                         ▼
     │                  [mavlink_tap UART1 RX]
     │                         ▼
     │              CRC (295 msgid table)
     │                         ▼
     │              filter HEARTBEAT (log only)
     │                         ▼
     │              [mavlink_format]
     │                         ▼
     │              [log_ring] ──► WebSocket ──► browser
```

---

## 4. Режими та стани

### 4.1. Машина станів

| Стан | Після power-on | Міст | UART | Лог | Редагування P1–P5, P7–P8 |
|------|----------------|------|------|-----|---------------------------|
| **RUN** | Так (за замовчуванням) | On (якщо P5) | Active | Оновлюється | Заблоковано |
| **STOP** | Після STOP | Off | Idle | Заморожений | Дозволено |
| **EXIT** | Після Exit | Off | Off | — | — → deep sleep |

Після **EXIT** або thermal shutdown — лише **новий цикл живлення** повертає пристрій у RUN.

### 4.2. Кнопки переходу (веб)

- **START** (з STOP): `settings::applyPending()`, `bridge::start(baud)`, WebSocket `state: RUN`.
- **STOP**: зупинка bridge, `state: STOP`.
- **Exit**: `shutdown::enterDeepSleep()`, повідомлення клієнтам WebSocket, Wi-Fi off.

### 4.3. Reboot

`POST /api/reboot` → `esp_restart()`. Застосовуються збережені в NVS SSID/пароль та параметри після попереднього **Save**.

---

## 5. Wi-Fi

| Параметр | Значення |
|----------|----------|
| Режим | SoftAP |
| SSID / пароль за замовчуванням | `MavLink-Analyzer` / `12345678` |
| IP AP | `192.168.4.1` |
| Канал | 6 |
| Макс. клієнтів | **1** |
| Захист | WPA2-PSK |

Зміна P7–P8: **Save** → **Reboot**.

---

## 6. Налаштування та NVS

### 6.1. Параметри P1–P8

| ID | Ключ NVS | Тип | За замовчуванням | Застосування |
|----|----------|-----|------------------|--------------|
| P1 | `proto` | enum | `auto` | На **START** |
| P2 | `baud` | uint32 | 115200 | На **START** (UART) |
| P3 | `parse` | bool | Off | На **START** |
| P4 | `filt_hb` | bool | On | На **START** |
| P5 | `pass` | bool | On | На **START** |
| P6 | — | — | Off | Лише браузер (localStorage) |
| P7 | `ssid` | string | `MavLink-Analyzer` | Після **Reboot** |
| P8 | `wpass` | string | `12345678` | Після **Reboot** |

Додатково в NVS: `testpkt` (bool) — для збірки **dev** (емуляція); у **release** не використовується.

### 6.2. API налаштувань

- `GET /api/settings` — `state`, `editable`, `active`, `pending`, `nvs_ok`, `test_packets_supported`.
- `POST /api/settings` — JSON pending (лише в **STOP**).
- `POST /api/save` — запис pending у NVS.
- `POST /api/reset` — заводські P1–P8 у NVS.

---

## 7. MAVLink: міст, CRC, лог, парсинг

### 7.1. Міст (bridge)

- Двостороння пересилка з буфером читання до 256 байт за виклик `poll()`.
- RX buffer UART: 1024 байт на порт.
- **Passthrough Off**: UART не пересилає; лог не оновлюється (немає джерела на UART1 RX через міст).
- Цикл main: `bridge::poll()` кожні ~1 ms у стані RUN.

### 7.2. Перевірка CRC

- Таблиця **295** записів: MAVLink **common** + **ardupilotmega** (реалізація та таблиця не в публічному репозиторії).
- Додатковий lookup у dialect **analyzer** для msgid з custom XML.
- Невідомий msgid: кадр **не** приймається в лог (сувора валідація).
- Підтримка **MAVLink v1** (STX `0xFE`) та **v2** (STX `0xFD`).

### 7.3. mavlink_tap

- Окремий state machine на **UART1 RX** (лог) і **UART0 RX** (telemetry count).
- Після валідного CRC: перевірка P1 (v1/v2/auto), фільтр msgid 0 (P4), форматування, `log_ring::push`, `web_server::onLogLine`.
- Лічильники: `frames_accepted`, `frames_crc_error`, `frames_filtered`.

### 7.4. Буфер логу

| Параметр | Значення |
|----------|----------|
| Ємність | 100 MAVLink-кадрів |
| Макс. текст на кадр | **768** символів (`LOG_ENTRY_MAX_CHARS`) |
| Тип | Кільцевий буфер (`log_ring`) |
| Переповнення | Видалення найстарішого запису |

### 7.5. Режим Parsing Off (raw)

Рядок містить: uptime ms, MAV v1/v2, **RCVR→FC**, sysid, compid, msgid, seq, len, **повний payload hex** (в межах 768 символів запису).

### 7.6. Режим Parsing On (parsed)

Структурований вивід для msgid з `analyzer.xml`:

| msgid | Повідомлення |
|-------|----------------|
| 0 | HEARTBEAT |
| 24 | GPS_RAW_INT |
| 30 | ATTITUDE |
| 33 | GLOBAL_POSITION_INT |
| 74 | VFR_HUD |
| 75 | COMMAND_INT (усі поля + імена MAV_CMD) |
| 76 | COMMAND_LONG |
| 152 | MEMINFO |
| 163 | AHRS |
| 253 | STATUSTEXT |

Інші валідні CRC-коректні кадри — **raw** як у §7.5.

### 7.7. Фільтр HEARTBEAT

- **Filter On**: msgid 0 не додається в `log_ring`; міст пересилає HEARTBEAT.
- **Filter Off**: HEARTBEAT у логу (parsed або raw).

### 7.8. Статистика (`stats` + `/api/status`)

| Метрика | Опис |
|---------|------|
| `uptime_ms` / `uptime_s` | Час від boot |
| `stats.log_frames_per_s` | Валідні кадри логу / с |
| `stats.telemetry_frames_per_s` | Кадри FC→RCVR / с |
| `stats.bridge_bytes_per_s` | Байти міста / с |
| `stats.crc_errors_per_s` | Помилки CRC на UART1 RX / с |
| `*_total` | Накопичувальні лічильники сесії |
| `mavlink.frames_accepted` | Всього прийнято в лог |
| `mavlink.frames_crc_error` | Всього CRC помилок |
| `mavlink.log_count` | Поточна кількість у буфері |
| `bridge_active`, `bridge_bytes_*` | Стан і байти міста |

Оновлення UI stats: poll **5 s** + події WebSocket `state`.

---

## 8. Веб-інтерфейс та API

### 8.1. Сторінка `/` (PROGMEM `web_page.h`)

- Адаптивна вёрстка (телефон / desktop), темна тема.
- Мови: **EN / UA** (`localStorage` `mavlink_lang`).
- **Debug** (`localStorage` `mavlink_debug`): блоки Memory, raw status JSON; підказки WebSocket.

Секції: керування (RUN/STOP/…), Live stats, Settings P1–P8, Debug tools, MAVLink log (`<pre>`).

### 8.2. WebSocket `/ws`

| Тип JSON | Зміст |
|----------|--------|
| `banner` | Рядок сесії: версія, baud, MAVLink, parsing, passthrough, filter_hb |
| `history` | Один пакет: до 30 останніх рядків текстом |
| `line` | Новий рядок логу |
| `state` | `RUN` / `STOP` |
| `exit` | Повідомлення завершення сесії |

При підключенні: banner + history snapshot + state. Захист від переповнення черги AsyncWS (`queueIsFull`).

### 8.3. REST API

| Метод | Шлях | Опис |
|-------|------|------|
| GET | `/` | HTML UI |
| GET | `/api/status` | JSON: firmware, state, uptime, bridge, settings, mavlink, stats, wifi, thermal, storage |
| GET | `/api/log` | JSON: `count`, `capacity`, `lines[]` |
| POST | `/api/start` | → RUN |
| POST | `/api/stop` | → STOP |
| GET/POST | `/api/settings` | Читання / запис pending |
| POST | `/api/save` | NVS |
| POST | `/api/reset` | Заводські |
| POST | `/api/reboot` | Restart |
| POST | `/api/exit` | Deep sleep |
| GET | `/api/save-log` | Завантаження `MavLinkLog_N.txt` |
| POST | `/api/debug` | Збірка dev: `test_packets` (у release не застосовується) |

### 8.4. Save Log

- Імена: `MavLinkLog_0.txt`, `MavLinkLog_1.txt`, …
- Лічильник у RAM, скидання після power-on.
- UTF-8, вміст = поточний буфер log_ring.

### 8.5. Browser log (P6)

- `localStorage`: `mavlink_browser_full_log`, `mavlink_browser_full_log_on`.
- Кнопка **Browser log** → `MavLinkBrowserLog.txt`.
- Обробка переповнення quota в UI.

---

## 9. USB Serial

| Подія | Рядок |
|-------|--------|
| Успішний boot | `BOOT OK` |
| NVS помилка (продовжує роботу) | `ERR_NVS` + `BOOT OK` |
| Wi-Fi / Web / UART init fail | `ERR_WIFI`, `ERR_WEB`, `ERR_UART` |

`monitor_speed` = 115200. MAVLink на USB **не** виводиться.

---

## 10. Температура та завершення роботи

| Поріг | Дія |
|-------|-----|
| ≥ **80 °C** (die) | WARN у UI (`/api/status` thermal.warn), latch з гістерезисом clear **72 °C** |
| ≥ **85 °C** | Emergency: Exit → deep sleep |
| Poll | Кожні **10 s** |

---

## 11. Збірка та ресурси

### 11.1. PlatformIO

| Environment | Призначення | `BRIDGE_SIMULATION` | Версія |
|-------------|-------------|---------------------|--------|
| **release** (default) | Польове використання, UART | 0 | 1.0.1 |
| **dev** | Розробка, Test packets у Debug | 1 | 1.0.1 |

```bash
pio run -e release -t upload
```

### 11.2. Ресурси release (орієнтовно)

| Ресурс | Використання |
|--------|----------------|
| Flash | ~884 KB (~67.5 % of 1.25 MB app) |
| RAM static | ~125 KB (~38 %) |
| OTA запас | ~426 KB flash вільно |

---

## 12. Реалізовані можливості v1.0.1

- Двосторонній UART-міст 8N1, baud 9600–921600 з NVS.
- Tap і лог **лише RCVR→FC**; телеметрія FC→RCVR у stats.
- CRC **295** msgid (common + ArduPilotMega).
- Кільцевий лог **100×768** символів; WebSocket real-time.
- Raw і parsed (10 типів повідомлень); fallback raw.
- Фільтр HEARTBEAT для логу.
- NVS P1–P8, Save, Reset, Reboot.
- SoftAP, 1 клієнт, http://192.168.4.1.
- RUN/STOP/EXIT, deep sleep, thermal protection.
- Save Log / Browser log (P6).
- UI EN/UA, Debug memory/status.
- `GET /api/status` з storage (heap, flash, NVS).
- Release без інжекції пакетів у лінію FC↔RCVR.

---

## Історія документа

| Версія | Дата | Зміни |
|--------|------|-------|
| 1.0.1 | 2026-05-16 | Специфікація реалізованого release v1.0.1 |

---

*Кінець документа*
