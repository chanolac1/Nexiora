#ifndef NEXIORA_NCP_MEMORY_NXMEMORY_H
#define NEXIORA_NCP_MEMORY_NXMEMORY_H

#include <stddef.h>
#include <stdint.h>
#include "Nexiora/NCP/Common/NxResult.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NX_MEMORY_DEFAULT_ALIGNMENT ((size_t)64)

typedef struct NxMemoryStats {
    uint64_t allocation_count;
    uint64_t free_count;
    uint64_t failed_allocation_count;
    size_t bytes_current;
    size_t bytes_peak;
    size_t bytes_total_allocated;
    size_t bytes_total_freed;
} NxMemoryStats;

typedef struct NxArenaStats {
    size_t capacity_bytes;
    size_t used_bytes;
    size_t peak_bytes;
    size_t remaining_bytes;
} NxArenaStats;

typedef struct NxArena {
    unsigned char* buffer;
    size_t capacity_bytes;
    size_t offset_bytes;
    size_t peak_bytes;
    size_t alignment_bytes;
} NxArena;

NxResult nx_memory_initialize(void);
void nx_memory_shutdown(void);

void* nx_memory_allocate(size_t size_bytes);
void* nx_memory_allocate_zeroed(size_t count, size_t size_bytes);
void* nx_memory_allocate_aligned(size_t size_bytes, size_t alignment_bytes);
void* nx_memory_allocate_aligned_zeroed(size_t size_bytes, size_t alignment_bytes);
void nx_memory_free(void* pointer);

int nx_memory_is_aligned(const void* pointer, size_t alignment_bytes);
NxMemoryStats nx_memory_get_stats(void);
void nx_memory_print_stats(void);

NxResult nx_arena_create(NxArena* arena, size_t capacity_bytes, size_t alignment_bytes);
void nx_arena_destroy(NxArena* arena);
void nx_arena_reset(NxArena* arena);
void* nx_arena_allocate(NxArena* arena, size_t size_bytes);
void* nx_arena_allocate_zeroed(NxArena* arena, size_t size_bytes);
NxArenaStats nx_arena_get_stats(const NxArena* arena);

#ifdef __cplusplus
}
#endif

#endif
