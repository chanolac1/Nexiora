#include "Nexiora/NCP/Memory/NxMemory.h"
#include <stdlib.h>
#include <string.h>

#define NX_MEMORY_MAGIC 0x4E584D45u

typedef struct NxMemoryHeader {
    uint32_t magic;
    size_t size_bytes;
} NxMemoryHeader;

static NxMemoryStats g_stats;

NxResult nx_memory_initialize(void) {
    memset(&g_stats, 0, sizeof(g_stats));
    return NX_OK;
}

void nx_memory_shutdown(void) {
}

void* nx_memory_allocate(size_t size_bytes) {
    if (size_bytes == 0) {
        return NULL;
    }

    NxMemoryHeader* header = (NxMemoryHeader*)malloc(sizeof(NxMemoryHeader) + size_bytes);
    if (!header) {
        return NULL;
    }

    header->magic = NX_MEMORY_MAGIC;
    header->size_bytes = size_bytes;

    g_stats.allocation_count++;
    g_stats.bytes_current += size_bytes;
    if (g_stats.bytes_current > g_stats.bytes_peak) {
        g_stats.bytes_peak = g_stats.bytes_current;
    }

    return (void*)(header + 1);
}

void* nx_memory_allocate_zeroed(size_t count, size_t size_bytes) {
    if (count == 0 || size_bytes == 0) {
        return NULL;
    }

    if (count > ((size_t)-1) / size_bytes) {
        return NULL;
    }

    size_t total = count * size_bytes;
    void* pointer = nx_memory_allocate(total);
    if (pointer) {
        memset(pointer, 0, total);
    }
    return pointer;
}

void nx_memory_free(void* pointer) {
    if (!pointer) {
        return;
    }

    NxMemoryHeader* header = ((NxMemoryHeader*)pointer) - 1;
    if (header->magic != NX_MEMORY_MAGIC) {
        return;
    }

    g_stats.free_count++;
    if (g_stats.bytes_current >= header->size_bytes) {
        g_stats.bytes_current -= header->size_bytes;
    } else {
        g_stats.bytes_current = 0;
    }

    header->magic = 0;
    free(header);
}

NxMemoryStats nx_memory_get_stats(void) {
    return g_stats;
}
