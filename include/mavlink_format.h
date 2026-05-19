#pragma once

#include <cstddef>
#include <cstdint>

/**
 * Format MAVLink log lines: parsed (P3) or raw fallback (TZ §7.5–7.6).
 */

namespace mavlink_format {

/**
 * Write one log entry into @p out.
 * @return bytes written (excluding NUL); 0 on error.
 * @p used_parsed set true when structured parse was used (75/76).
 */
size_t formatLogEntry(const uint8_t* frame, size_t frame_len, uint8_t mavlink_version, char* out,
                      size_t out_len, bool* used_parsed);

}  // namespace mavlink_format
