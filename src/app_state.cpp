#include "app_state.h"

namespace app_state {

namespace {

AppState current = AppState::Run;

bool canStartFrom(AppState s) {
    return s == AppState::Stop;
}

bool canStopFrom(AppState s) {
    return s == AppState::Run;
}

}  // namespace

void init() {
    // TZ §4: power-on enters RUN
    current = AppState::Run;
}

AppState get() {
    return current;
}

const char* name(AppState state) {
    switch (state) {
        case AppState::Run:
            return "RUN";
        case AppState::Stop:
            return "STOP";
        case AppState::Exit:
            return "EXIT";
    }
    return "UNKNOWN";
}

bool isRun() {
    return current == AppState::Run;
}

bool isStop() {
    return current == AppState::Stop;
}

bool isExit() {
    return current == AppState::Exit;
}

bool requestStart() {
    if (!canStartFrom(current)) {
        return false;
    }
    current = AppState::Run;
    return true;
}

bool requestStop() {
    if (!canStopFrom(current)) {
        return false;
    }
    current = AppState::Stop;
    return true;
}

bool requestExit() {
    if (current == AppState::Exit) {
        return false;
    }
    current = AppState::Exit;
    return true;
}

}  // namespace app_state
