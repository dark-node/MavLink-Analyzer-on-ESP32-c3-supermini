#pragma once

#include <cstdint>

/**
 * Device state machine: RUN / STOP / EXIT (TZ §4).
 */

enum class AppState : uint8_t {
    Run = 0,
    Stop,
    Exit,
};

namespace app_state {

void init();

AppState get();
const char* name(AppState state);

bool isRun();
bool isStop();
bool isExit();

/** STOP → RUN; applies only from Stop. Returns false if transition rejected. */
bool requestStart();

/** RUN → STOP. Returns false if transition rejected. */
bool requestStop();

/** Any → Exit (terminal until power cycle / deep sleep in later phase). */
bool requestExit();

}  // namespace app_state
