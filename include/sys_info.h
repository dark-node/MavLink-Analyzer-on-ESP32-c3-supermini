#pragma once

#include <cstddef>
#include <cstdint>

/**
 * Runtime memory / storage snapshot (RAM, flash, NVS, static buffers).
 */

namespace sys_info {

struct Snapshot {
    uint32_t heap_free;
    uint32_t heap_min_free;
    uint32_t heap_largest_block;
    uint32_t heap_total;

    uint32_t flash_chip_size;
    uint32_t sketch_size;
    uint32_t sketch_free;

    uint32_t log_buffer_bytes;

    uint32_t nvs_used_entries;
    uint32_t nvs_free_entries;
    uint32_t nvs_namespace_count;
    bool nvs_stats_ok;
};

void refresh();
const Snapshot& snapshot();

/** Append JSON object fields for "storage":{...} (no outer braces). */
int appendStorageJson(char* out, size_t out_len);

}  // namespace sys_info
