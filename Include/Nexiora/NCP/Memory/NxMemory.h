#ifndef NEXIORA_NCP_MEMORY_NXMEMORY_H
#define NEXIORA_NCP_MEMORY_NXMEMORY_H

#include <stddef.h>
#include <stdint.h>
#include "Nexiora/NCP/Common/NxResult.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxMemoryStats {
    uint64_t allocation_count;
    uint64_t free_count;
    size_t bytes_current;
    size_t bytes_peak;
} NxMemoryStats;

NxResult nx_memory_initialize(void);
void nx_memory_shutdown(void);
void* nx_memory_allocate(size_t size_bytes);
void* nx_memory_allocate_zeroed(size_t count, size_t size_bytes);
void nx_memory_free(void* pointer);
NxMemoryStats nx_memory_get_stats(void);

#ifdef __cplusplus
}
#endif

#endif
