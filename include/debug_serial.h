#pragma once

/**
 * USB CDC: BOOT OK / ERR_* on boot (TZ §9).
 * Periodic TEMP lines are printed by thermal.cpp (user safety).
 */

namespace debug_serial {

void begin();
void bootOk();
void error(const char* code);

}  // namespace debug_serial
