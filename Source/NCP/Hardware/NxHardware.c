#include "Nexiora/NCP/Hardware/NxHardware.h"
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#endif

NxResult nx_hardware_query(NxHardwareInfo* info) {
    if (!info) {
        return NX_ERROR_ARGUMENT;
    }

    memset(info, 0, sizeof(*info));

#if defined(_WIN32)
    strcpy_s(info->platform, sizeof(info->platform), "Windows x64");
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    info->logical_processors = sys_info.dwNumberOfProcessors;

    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(mem_status);
    if (GlobalMemoryStatusEx(&mem_status)) {
        info->total_memory_bytes = mem_status.ullTotalPhys;
    }

    int cpu_info[4] = {0};
    __cpuid(cpu_info, 1);
    info->has_sse2 = (cpu_info[3] & (1 << 26)) != 0;
    int avx_supported = (cpu_info[2] & (1 << 28)) != 0;

    __cpuid(cpu_info, 7);
    info->has_avx2 = avx_supported && ((cpu_info[1] & (1 << 5)) != 0);
    info->has_avx512f = avx_supported && ((cpu_info[1] & (1 << 16)) != 0);
#else
    strncpy(info->platform, "Generic", sizeof(info->platform) - 1);
    info->logical_processors = 1;
#endif

    return NX_OK;
}

void nx_hardware_print(const NxHardwareInfo* info) {
    if (!info) {
        return;
    }
    printf("Nexiora Hardware\n");
    printf("  Platform: %s\n", info->platform);
    printf("  Logical processors: %u\n", info->logical_processors);
    printf("  Total memory: %.2f GB\n", (double)info->total_memory_bytes / (1024.0 * 1024.0 * 1024.0));
    printf("  SSE2: %s\n", info->has_sse2 ? "yes" : "no");
    printf("  AVX2: %s\n", info->has_avx2 ? "yes" : "no");
    printf("  AVX512F: %s\n", info->has_avx512f ? "yes" : "no");
}
