#include "log_ring.h"

#include "config.h"

#include <cstring>

namespace log_ring {

namespace {

char buffer[LOG_RING_CAPACITY][LOG_ENTRY_MAX_CHARS];
size_t head = 0;
size_t count_entries = 0;

uint32_t stat_accepted = 0;
uint32_t stat_crc_error = 0;
uint32_t stat_filtered = 0;

}  // namespace

void init() {
    clear();
    stat_accepted = 0;
    stat_crc_error = 0;
    stat_filtered = 0;
}

void clear() {
    head = 0;
    count_entries = 0;
    for (size_t i = 0; i < LOG_RING_CAPACITY; ++i) {
        buffer[i][0] = '\0';
    }
}

void push(const char* line) {
    if (line == nullptr) {
        return;
    }

    strncpy(buffer[head], line, LOG_ENTRY_MAX_CHARS - 1);
    buffer[head][LOG_ENTRY_MAX_CHARS - 1] = '\0';

    head = (head + 1) % LOG_RING_CAPACITY;
    if (count_entries < LOG_RING_CAPACITY) {
        ++count_entries;
    }
}

size_t count() {
    return count_entries;
}

size_t capacity() {
    return LOG_RING_CAPACITY;
}

size_t getLine(size_t index, char* out, size_t out_len) {
    if (out == nullptr || out_len == 0 || index >= count_entries) {
        return 0;
    }

    const size_t start = (head + LOG_RING_CAPACITY - count_entries) % LOG_RING_CAPACITY;
    const size_t slot = (start + index) % LOG_RING_CAPACITY;

    strncpy(out, buffer[slot], out_len - 1);
    out[out_len - 1] = '\0';
    return strlen(out);
}

uint32_t framesAccepted() {
    return stat_accepted;
}

uint32_t framesCrcError() {
    return stat_crc_error;
}

uint32_t framesFiltered() {
    return stat_filtered;
}

void recordAccepted() {
    ++stat_accepted;
}

void recordCrcError() {
    ++stat_crc_error;
}

void recordFiltered() {
    ++stat_filtered;
}

}  // namespace log_ring
