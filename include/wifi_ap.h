#pragma once

#include <cstdint>

/**
 * Wi-Fi SoftAP for web UI (TZ §5). No STA / internet.
 */

namespace wifi_ap {

bool init();

/** Start AP with default SSID/password from config.h. */
bool start();

bool start(const char* ssid, const char* password);

void stop();

bool isActive();

const char* ssid();
const char* ipAddress();

/** Number of stations currently associated (0 or 1). */
uint8_t connectedClients();

}  // namespace wifi_ap
