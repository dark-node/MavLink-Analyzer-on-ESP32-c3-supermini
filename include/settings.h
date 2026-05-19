#pragma once

#include "config.h"

#include <cstddef>
#include <cstdint>

/**
 * NVS settings P1–P8 (TZ §6). Pending values apply on START; Wi‑Fi on Reboot.
 */

enum class MavlinkProtocol : uint8_t {
    Auto = 0,
    V1,
    V2,
};

namespace settings {

bool init();
bool nvsOk();

/** Active (running) parameters. */
MavlinkProtocol mavlinkProtocol();
const char* mavlinkProtocolName(MavlinkProtocol p);
uint32_t baudrate();
bool parsing();
bool filterHeartbeat();
bool passthrough();
/** Synthetic MAVLink inject (desk test). Requires BRIDGE_SIMULATION build flag. */
bool testPackets();
const char* wifiSsid();
const char* wifiPassword();

/** Pending (form / next START). */
MavlinkProtocol pendingMavlinkProtocol();
uint32_t pendingBaudrate();
bool pendingParsing();
bool pendingFilterHeartbeat();
bool pendingPassthrough();
bool pendingTestPackets();
const char* pendingWifiSsid();
const char* pendingWifiPassword();

void setPendingMavlinkProtocol(MavlinkProtocol p);
void setPendingBaudrate(uint32_t baud);
void setPendingParsing(bool on);
void setPendingFilterHeartbeat(bool on);
void setPendingPassthrough(bool on);
void setPendingTestPackets(bool on);
void setPendingWifiSsid(const char* ssid);
void setPendingWifiPassword(const char* password);

/** Copy pending → active (call on START). */
void applyPending();

/** Apply test_packets immediately (active + pending + NVS). Restarts bridge if RUN. */
bool setTestPackets(bool on);

#if BRIDGE_SIMULATION
constexpr bool testPacketsSupported() { return true; }
#else
constexpr bool testPacketsSupported() { return false; }
#endif

/** Write pending to NVS (Зберегти). */
bool saveToNvs();

/** Factory defaults → pending + NVS. */
bool resetToFactory();

bool isValidBaudrate(uint32_t baud);

/** Parse JSON body into pending; returns false on error. */
bool updatePendingFromJson(const char* json, size_t len, char* err, size_t err_len);

/** Debug API: currently only test_packets (applied immediately). */
bool updateDebugFromJson(const char* json, size_t len, char* err, size_t err_len);

}  // namespace settings
