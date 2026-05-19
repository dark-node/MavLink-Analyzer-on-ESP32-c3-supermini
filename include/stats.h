#pragma once

#include <cstdint>

/** Live rates and counters (TZ §7.8). */

namespace stats {

struct Snapshot {
    float log_frames_per_s;
    float telemetry_frames_per_s;
    float bridge_bytes_per_s;
    float crc_errors_per_s;
    uint32_t log_frames_total;
    uint32_t telemetry_frames_total;
    uint32_t crc_errors_total;
    uint32_t bridge_bytes_total;
};

void init();
void poll();

void onLogFrame();
void onCrcError();
void onTelemetryFrame();

Snapshot snapshot();

}  // namespace stats
