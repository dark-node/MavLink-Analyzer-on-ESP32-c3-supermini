#pragma once

/**
 * MavLink Analyzer — hardware and application limits (TZ v1.0.1).
 * Pinout: ESP32-C3 Super Mini
 */

// -----------------------------------------------------------------------------
// Firmware identity
// -----------------------------------------------------------------------------
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "1.0.1"
#endif

// -----------------------------------------------------------------------------
// UART0 — flight controller side
// -----------------------------------------------------------------------------
#define UART_FC_NUM 0
#define PIN_UART_FC_RX 20
#define PIN_UART_FC_TX 21

// -----------------------------------------------------------------------------
// UART1 — receiver side
// -----------------------------------------------------------------------------
#define UART_RCVR_NUM 1
#define PIN_UART_RCVR_RX 4
#define PIN_UART_RCVR_TX 5

// -----------------------------------------------------------------------------
// Serial (USB CDC service port, not MAVLink)
// -----------------------------------------------------------------------------
#define USB_SERIAL_BAUD 115200

/** Max wait for USB CDC host (Monitor) before BOOT OK / continuing boot */
#define USB_SERIAL_HOST_WAIT_MS 3000

// -----------------------------------------------------------------------------
// UART data format (both ports)
// -----------------------------------------------------------------------------
#define UART_DATA_BITS SERIAL_8N1

// Default baudrate (NVS will override in a later phase)
#define DEFAULT_BAUDRATE 115200

// Allowed baudrates for settings UI (later phase)
#define BAUDRATE_LIST 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600

// -----------------------------------------------------------------------------
// Log buffer (ESP)
// -----------------------------------------------------------------------------
#define LOG_RING_CAPACITY 100

// Maximum formatted text per log entry (parsed may use multiple lines)
#define LOG_ENTRY_MAX_CHARS 768

// -----------------------------------------------------------------------------
// Wi-Fi SoftAP (defaults; NVS in later phase)
// -----------------------------------------------------------------------------
#define WIFI_AP_SSID_DEFAULT "MavLink-Analyzer"
#define WIFI_AP_PASS_DEFAULT "12345678"
#define WIFI_AP_MAX_CLIENTS 1
#define WIFI_AP_CHANNEL 6
#define WIFI_AP_IP_OCTET_1 192
#define WIFI_AP_IP_OCTET_2 168
#define WIFI_AP_IP_OCTET_3 4
#define WIFI_AP_IP_OCTET_4 1

// -----------------------------------------------------------------------------
// Bridge test-packet support (compile sim inject code; runtime toggle in web Debug)
// Override in platformio.ini: -DBRIDGE_SIMULATION=0 for release / no sim code
// -----------------------------------------------------------------------------
#ifndef BRIDGE_SIMULATION
#define BRIDGE_SIMULATION 1
#endif

#ifndef BRIDGE_SIM_TRACE
#define BRIDGE_SIM_TRACE 1
#endif

/** Sim RCVR→FC inject interval (desk test; reduce ESP/Wi‑Fi load). */
#define BRIDGE_SIM_INJECT_MS 2000
#define BRIDGE_SIM_FC_TELEMETRY_MS 2000
#define BRIDGE_SIM_QUEUE_MAX 384

// -----------------------------------------------------------------------------
// Chip temperature monitor (user safety; internal ESP32-C3 die sensor)
// -----------------------------------------------------------------------------
#define THERMAL_MONITOR_INTERVAL_MS 10000
/** Die temperature WARN (°C). ESP32-C3 runs hot under Wi‑Fi; not ambient air. */
#define THERMAL_WARN_CELSIUS 80.0f
/** Hysteresis: clear WARN latch below this (°C). */
#define THERMAL_WARN_CLEAR_CELSIUS 72.0f
/** Emergency shutdown: bridge + Wi-Fi off, deep sleep (°C). */
#define THERMAL_SHUTDOWN_CELSIUS 85.0f

// -----------------------------------------------------------------------------
// Timing
// -----------------------------------------------------------------------------
#define USB_SERIAL_STARTUP_MS 500
#define MAIN_LOOP_DELAY_MS 1
