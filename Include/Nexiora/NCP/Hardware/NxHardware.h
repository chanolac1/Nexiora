#ifndef NEXIORA_NCP_HARDWARE_NXHARDWARE_H
#define NEXIORA_NCP_HARDWARE_NXHARDWARE_H

#include <stdint.h>
#include "Nexiora/NCP/Common/NxResult.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NX_HARDWARE_VENDOR_LENGTH 16
#define NX_HARDWARE_BRAND_LENGTH 64
#define NX_HARDWARE_PLATFORM_LENGTH 64

typedef enum NxHardwareVendor {
    NX_HARDWARE_VENDOR_UNKNOWN = 0,
    NX_HARDWARE_VENDOR_INTEL,
    NX_HARDWARE_VENDOR_AMD,
    NX_HARDWARE_VENDOR_ARM,
    NX_HARDWARE_VENDOR_APPLE
} NxHardwareVendor;

typedef struct NxHardwareInfo {
    char platform[NX_HARDWARE_PLATFORM_LENGTH];
    char cpu_vendor_string[NX_HARDWARE_VENDOR_LENGTH];
    char cpu_brand_string[NX_HARDWARE_BRAND_LENGTH];
    NxHardwareVendor cpu_vendor;

    uint32_t logical_processors;
    uint32_t physical_cores;
    uint64_t total_memory_bytes;
    uint64_t available_memory_bytes;

    uint32_t cache_line_bytes;
    uint32_t l1_data_cache_bytes;
    uint32_t l2_cache_bytes;
    uint32_t l3_cache_bytes;

    int has_mmx;
    int has_sse;
    int has_sse2;
    int has_sse3;
    int has_ssse3;
    int has_sse41;
    int has_sse42;
    int has_fma;
    int has_aes;
    int has_avx;
    int has_avx2;
    int has_avx512f;
    int has_avx512dq;
    int has_avx512cd;
    int has_avx512bw;
    int has_avx512vl;
    int has_bmi1;
    int has_bmi2;
    int has_popcnt;
} NxHardwareInfo;

NxResult nx_hardware_query(NxHardwareInfo* info);
void nx_hardware_print(const NxHardwareInfo* info);
const char* nx_hardware_vendor_to_string(NxHardwareVendor vendor);
int nx_hardware_supports_any_simd(const NxHardwareInfo* info);

#ifdef __cplusplus
}
#endif

#endif
