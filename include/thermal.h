#pragma once

/**
 * Internal chip temperature monitor (ESP32-C3 die sensor).
 * User safety feature — not part of TZ v1 web UI.
 */

namespace thermal {

void init();

/** Call every main loop iteration; reports every THERMAL_MONITOR_INTERVAL_MS. */
void poll();

float lastCelsius();

bool isWarning();
bool hasEmergencyShutdown();

}  // namespace thermal
