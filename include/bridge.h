#pragma once

#include <cstdint>

/**
 * Transparent UART bridge: UART0 (FC) <-> UART1 (receiver), TZ §7.1.
 * Test packets (settings::testPackets): synthetic inject when BRIDGE_SIMULATION is compiled in.
 * Otherwise UART0 (FC) <-> UART1 (receiver).
 */

namespace bridge {

bool init();
bool start(uint32_t baud);
void stop();
bool isActive();

/** True when firmware runs without real UART bridge (simulation build). */
bool isSimulation();

/** Forward pending bytes (UART or sim queues). */
void poll();

/** Cumulative forwarded bytes since last start() (both directions). */
uint32_t bytesForwardedFcToRcvr();
uint32_t bytesForwardedRcvrToFc();

}  // namespace bridge
