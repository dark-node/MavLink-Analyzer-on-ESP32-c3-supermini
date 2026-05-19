#pragma once

#include <cstddef>
#include <cstdint>

/**
 * MAVLink tap on RCVR->FC byte stream only (UART1 RX direction), TZ §7.2.
 */

namespace mavlink_tap {

void init();
void reset();

/** Feed one byte from receiver toward FC (bridge tap). */
void feedByte(uint8_t byte);

/** Feed buffer (e.g. simulation batch). */
void feedBuffer(const uint8_t* data, size_t len);

/** Count valid MAVLink frames on FC→RCVR (telemetry), no logging (TZ §7.8). */
void feedTelemetryBuffer(const uint8_t* data, size_t len);

uint32_t framesAccepted();
uint32_t framesCrcErrors();

}  // namespace mavlink_tap
