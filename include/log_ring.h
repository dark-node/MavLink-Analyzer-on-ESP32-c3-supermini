#pragma once

#include <cstddef>
#include <cstdint>

namespace log_ring {

void init();
void clear();

/** Push one formatted log line (one MAVLink frame). */
void push(const char* line);

size_t count();
size_t capacity();

/** Copy line @p index (0 = oldest). Returns length or 0 if empty. */
size_t getLine(size_t index, char* out, size_t out_len);

uint32_t framesAccepted();
uint32_t framesCrcError();
uint32_t framesFiltered();

void recordAccepted();
void recordCrcError();
void recordFiltered();

}  // namespace log_ring
