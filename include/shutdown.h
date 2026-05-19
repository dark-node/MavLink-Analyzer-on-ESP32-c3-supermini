#pragma once

/**
 * Graceful shutdown: web off, bridge/UART off, Wi‑Fi off, deep sleep (TZ §4.4).
 */

namespace shutdown {

/** Stop services and enter deep sleep (does not return). */
void enterDeepSleep();

bool hasEnteredSleep();

}  // namespace shutdown
