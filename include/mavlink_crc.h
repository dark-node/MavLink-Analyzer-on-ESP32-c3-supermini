#pragma once

#include <cstddef>
#include <cstdint>

namespace mavlink_crc {

uint16_t accumulate(uint8_t byte, uint16_t crc);
uint16_t accumulateBuffer(const uint8_t* data, size_t len, uint16_t crc);

/** MAVLink CRC extra for msgid (common+APM table); 0 if unknown. */
uint8_t extraForMsgid(uint32_t msgid);

/** Append CRC to v2 frame in @p frame (STX..payload); returns total length. */
size_t finalizeV2Frame(uint8_t* frame, size_t payload_len, uint32_t msgid);

bool validateFrame(const uint8_t* frame, size_t frame_len);

}  // namespace mavlink_crc
