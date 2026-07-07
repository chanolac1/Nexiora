#ifndef NEXIORA_NCP_HARDWARE_NXHARDWARE_H
#define NEXIORA_NCP_HARDWARE_NXHARDWARE_H

#include <stdint.h>
#include "Nexiora/NCP/Common/NxResult.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxHardwareInfo {
    char platform[64];
    uint32_t logical_processors;
    uint64_t total_memory_bytes;
    int has_sse2;
    int has_avx2;
    int has_avx512f;
} NxHardwareInfo;

NxResult nx_hardware_query(NxHardwareInfo* info);
void nx_hardware_print(const NxHardwareInfo* info);

#ifdef __cplusplus
}
#endif

#endif
