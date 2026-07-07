#include "Nexiora/NCP/Memory/NxMemory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NX_MEMORY_MAGIC 0x4E584D45u
#define NX_MEMORY_FREED 0x46524545u

typedef struct NxMemoryHeader {
    uint32_t magic;
    uint32_t reserved;
    size_t size_bytes;
    void* raw_pointer;
} NxMemoryHeader;

static NxMemoryStats g_stats;

static int nx_is_power_of_two(size_t value) {
    return value != 0 && (value & (value - 1)) == 0;
}

static size_t nx_align_up_size(size_t value, size_t alignment) {
    return (value + alignment - 1u) & ~(alignment - 1u);
}

static uintptr_t nx_align_up_uintptr(uintptr_t value, size_t alignment) {
    uintptr_t mask = (uintptr_t)(alignment - 1u);
    return (value + mask) & ~mask;
}

NxResult nx_memory_initialize(void) {
    memset(&g_stats, 0, sizeof(g_stats));
    return NX_OK;
}

void nx_memory_shutdown(void) {
    /* Genesis policy: the manager exposes outstanding bytes through stats.
       Leak reporting will become stricter when the audit subsystem is added. */
}

void* nx_memory_allocate(size_t size_bytes) {
    return nx_memory_allocate_aligned(size_bytes, NX_MEMORY_DEFAULT_ALIGNMENT);
}

void* nx_memory_allocate_zeroed(size_t count, size_t size_bytes) {
    if (count == 0 || size_bytes == 0) {
        return NULL;
    }

    if (count > ((size_t)-1) / size_bytes) {
        g_stats.failed_allocation_count++;
        return NULL;
    }

    return nx_memory_allocate_aligned_zeroed(count * size_bytes, NX_MEMORY_DEFAULT_ALIGNMENT);
}

void* nx_memory_allocate_aligned(size_t size_bytes, size_t alignment_bytes) {
    if (size_bytes == 0) {
        return NULL;
    }

    if (alignment_bytes < sizeof(void*)) {
        alignment_bytes = sizeof(void*);
    }

    if (!nx_is_power_of_two(alignment_bytes)) {
        g_stats.failed_allocation_count++;
        return NULL;
    }

    const size_t header_bytes = sizeof(NxMemoryHeader);
    const size_t extra = alignment_bytes - 1u + header_bytes;

    if (size_bytes > ((size_t)-1) - extra) {
        g_stats.failed_allocation_count++;
        return NULL;
    }

    void* raw = malloc(size_bytes + extra);
    if (!raw) {
        g_stats.failed_allocation_count++;
        return NULL;
    }

    uintptr_t raw_addr = (uintptr_t)raw;
    uintptr_t user_addr = nx_align_up_uintptr(raw_addr + header_bytes, alignment_bytes);
    NxMemoryHeader* header = ((NxMemoryHeader*)user_addr) - 1;

    header->magic = NX_MEMORY_MAGIC;
    header->reserved = 0;
    header->size_bytes = size_bytes;
    header->raw_pointer = raw;

    g_stats.allocation_count++;
    g_stats.bytes_current += size_bytes;
    g_stats.bytes_total_allocated += size_bytes;
    if (g_stats.bytes_current > g_stats.bytes_peak) {
        g_stats.bytes_peak = g_stats.bytes_current;
    }

    return (void*)user_addr;
}

void* nx_memory_allocate_aligned_zeroed(size_t size_bytes, size_t alignment_bytes) {
    void* pointer = nx_memory_allocate_aligned(size_bytes, alignment_bytes);
    if (pointer) {
        memset(pointer, 0, size_bytes);
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
    g_stats.bytes_total_freed += header->size_bytes;
    if (g_stats.bytes_current >= header->size_bytes) {
        g_stats.bytes_current -= header->size_bytes;
    } else {
        g_stats.bytes_current = 0;
    }

    void* raw = header->raw_pointer;
    header->magic = NX_MEMORY_FREED;
    header->raw_pointer = NULL;
    header->size_bytes = 0;
    free(raw);
}

int nx_memory_is_aligned(const void* pointer, size_t alignment_bytes) {
    if (!pointer || alignment_bytes == 0 || !nx_is_power_of_two(alignment_bytes)) {
        return 0;
    }
    return (((uintptr_t)pointer) & (uintptr_t)(alignment_bytes - 1u)) == 0;
}

NxMemoryStats nx_memory_get_stats(void) {
    return g_stats;
}

void nx_memory_print_stats(void) {
    printf("NxMemory Stats\n");
    printf("  Allocations:        %llu\n", (unsigned long long)g_stats.allocation_count);
    printf("  Frees:              %llu\n", (unsigned long long)g_stats.free_count);
    printf("  Failed allocations: %llu\n", (unsigned long long)g_stats.failed_allocation_count);
    printf("  Current bytes:      %zu\n", g_stats.bytes_current);
    printf("  Peak bytes:         %zu\n", g_stats.bytes_peak);
    printf("  Total allocated:    %zu\n", g_stats.bytes_total_allocated);
    printf("  Total freed:        %zu\n", g_stats.bytes_total_freed);
}

NxResult nx_arena_create(NxArena* arena, size_t capacity_bytes, size_t alignment_bytes) {
    if (!arena || capacity_bytes == 0) {
        return NX_ERROR_ARGUMENT;
    }

    if (alignment_bytes < sizeof(void*)) {
        alignment_bytes = sizeof(void*);
    }

    if (!nx_is_power_of_two(alignment_bytes)) {
        return NX_ERROR_ARGUMENT;
    }

    memset(arena, 0, sizeof(*arena));
    arena->buffer = (unsigned char*)nx_memory_allocate_aligned(capacity_bytes, alignment_bytes);
    if (!arena->buffer) {
        return NX_ERROR_MEMORY;
    }

    arena->capacity_bytes = capacity_bytes;
    arena->alignment_bytes = alignment_bytes;
    return NX_OK;
}

void nx_arena_destroy(NxArena* arena) {
    if (!arena) {
        return;
    }
    nx_memory_free(arena->buffer);
    memset(arena, 0, sizeof(*arena));
}

void nx_arena_reset(NxArena* arena) {
    if (!arena) {
        return;
    }
    arena->offset_bytes = 0;
}

void* nx_arena_allocate(NxArena* arena, size_t size_bytes) {
    if (!arena || !arena->buffer || size_bytes == 0) {
        return NULL;
    }

    size_t aligned_offset = nx_align_up_size(arena->offset_bytes, arena->alignment_bytes);
    if (aligned_offset > arena->capacity_bytes || size_bytes > arena->capacity_bytes - aligned_offset) {
        return NULL;
    }

    void* pointer = arena->buffer + aligned_offset;
    arena->offset_bytes = aligned_offset + size_bytes;
    if (arena->offset_bytes > arena->peak_bytes) {
        arena->peak_bytes = arena->offset_bytes;
    }
    return pointer;
}

void* nx_arena_allocate_zeroed(NxArena* arena, size_t size_bytes) {
    void* pointer = nx_arena_allocate(arena, size_bytes);
    if (pointer) {
        memset(pointer, 0, size_bytes);
    }
    return pointer;
}

NxArenaStats nx_arena_get_stats(const NxArena* arena) {
    NxArenaStats stats;
    memset(&stats, 0, sizeof(stats));
    if (!arena) {
        return stats;
    }
    stats.capacity_bytes = arena->capacity_bytes;
    stats.used_bytes = arena->offset_bytes;
    stats.peak_bytes = arena->peak_bytes;
    stats.remaining_bytes = arena->capacity_bytes >= arena->offset_bytes
        ? arena->capacity_bytes - arena->offset_bytes
        : 0;
    return stats;
}
