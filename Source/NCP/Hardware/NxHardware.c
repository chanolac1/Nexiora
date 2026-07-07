#include "Nexiora/NCP/Hardware/NxHardware.h"
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#if defined(__x86_64__) || defined(__i386__)
#include <cpuid.h>
#endif
#endif

static void nx_hw_zero(NxHardwareInfo* info) {
    memset(info, 0, sizeof(*info));
    info->cache_line_bytes = 64;
}

static NxHardwareVendor nx_hw_detect_vendor(const char* vendor) {
    if (!vendor) return NX_HARDWARE_VENDOR_UNKNOWN;
    if (strcmp(vendor, "GenuineIntel") == 0) return NX_HARDWARE_VENDOR_INTEL;
    if (strcmp(vendor, "AuthenticAMD") == 0) return NX_HARDWARE_VENDOR_AMD;
    if (strstr(vendor, "ARM") != NULL) return NX_HARDWARE_VENDOR_ARM;
    if (strstr(vendor, "Apple") != NULL) return NX_HARDWARE_VENDOR_APPLE;
    return NX_HARDWARE_VENDOR_UNKNOWN;
}

const char* nx_hardware_vendor_to_string(NxHardwareVendor vendor) {
    switch (vendor) {
        case NX_HARDWARE_VENDOR_INTEL: return "Intel";
        case NX_HARDWARE_VENDOR_AMD: return "AMD";
        case NX_HARDWARE_VENDOR_ARM: return "ARM";
        case NX_HARDWARE_VENDOR_APPLE: return "Apple";
        default: return "Unknown";
    }
}

int nx_hardware_supports_any_simd(const NxHardwareInfo* info) {
    if (!info) return 0;
    return info->has_sse || info->has_sse2 || info->has_avx || info->has_avx2 || info->has_avx512f;
}

#if defined(_WIN32)
static void nx_cpuid(int out[4], int leaf, int subleaf) {
    __cpuidex(out, leaf, subleaf);
}

static uint64_t nx_xgetbv0(void) {
    return _xgetbv(0);
}
#elif defined(__x86_64__) || defined(__i386__)
static void nx_cpuid(int out[4], int leaf, int subleaf) {
    unsigned int a = 0, b = 0, c = 0, d = 0;
    __cpuid_count((unsigned int)leaf, (unsigned int)subleaf, a, b, c, d);
    out[0] = (int)a; out[1] = (int)b; out[2] = (int)c; out[3] = (int)d;
}

static uint64_t nx_xgetbv0(void) {
#if defined(__GNUC__) || defined(__clang__)
    unsigned int eax = 0, edx = 0;
    __asm__ volatile ("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    return ((uint64_t)edx << 32) | eax;
#else
    return 0;
#endif
}
#endif

#if defined(_WIN32) || defined(__x86_64__) || defined(__i386__)
static int nx_hw_has_leaf(int leaf) {
    int r[4] = {0,0,0,0};
    nx_cpuid(r, 0, 0);
    return r[0] >= leaf;
}

static int nx_hw_has_extended_leaf(int leaf) {
    int r[4] = {0,0,0,0};
    nx_cpuid(r, 0x80000000, 0);
    return r[0] >= leaf;
}

static void nx_hw_query_x86(NxHardwareInfo* info) {
    int r[4] = {0,0,0,0};

    nx_cpuid(r, 0, 0);
    memcpy(info->cpu_vendor_string + 0, &r[1], 4);
    memcpy(info->cpu_vendor_string + 4, &r[3], 4);
    memcpy(info->cpu_vendor_string + 8, &r[2], 4);
    info->cpu_vendor_string[12] = '\0';
    info->cpu_vendor = nx_hw_detect_vendor(info->cpu_vendor_string);

    if (nx_hw_has_extended_leaf(0x80000004)) {
        int brand[12];
        nx_cpuid(&brand[0], 0x80000002, 0);
        nx_cpuid(&brand[4], 0x80000003, 0);
        nx_cpuid(&brand[8], 0x80000004, 0);
        memcpy(info->cpu_brand_string, brand, 48);
        info->cpu_brand_string[48] = '\0';
    }

    if (nx_hw_has_leaf(1)) {
        nx_cpuid(r, 1, 0);
        info->has_mmx = (r[3] & (1 << 23)) != 0;
        info->has_sse = (r[3] & (1 << 25)) != 0;
        info->has_sse2 = (r[3] & (1 << 26)) != 0;
        info->has_sse3 = (r[2] & (1 << 0)) != 0;
        info->has_ssse3 = (r[2] & (1 << 9)) != 0;
        info->has_sse41 = (r[2] & (1 << 19)) != 0;
        info->has_sse42 = (r[2] & (1 << 20)) != 0;
        info->has_fma = (r[2] & (1 << 12)) != 0;
        info->has_aes = (r[2] & (1 << 25)) != 0;
        info->has_popcnt = (r[2] & (1 << 23)) != 0;

        int osxsave = (r[2] & (1 << 27)) != 0;
        int cpu_avx = (r[2] & (1 << 28)) != 0;
        int avx_os = 0;
        if (osxsave && cpu_avx) {
            uint64_t xcr0 = nx_xgetbv0();
            avx_os = ((xcr0 & 0x6) == 0x6);
        }
        info->has_avx = cpu_avx && avx_os;
    }

    if (nx_hw_has_leaf(7)) {
        nx_cpuid(r, 7, 0);
        info->has_bmi1 = (r[1] & (1 << 3)) != 0;
        info->has_avx2 = info->has_avx && ((r[1] & (1 << 5)) != 0);
        info->has_bmi2 = (r[1] & (1 << 8)) != 0;

        uint64_t xcr0 = nx_xgetbv0();
        int avx512_os = ((xcr0 & 0xE6) == 0xE6);
        info->has_avx512f = avx512_os && ((r[1] & (1 << 16)) != 0);
        info->has_avx512dq = avx512_os && ((r[1] & (1 << 17)) != 0);
        info->has_avx512cd = avx512_os && ((r[1] & (1 << 28)) != 0);
        info->has_avx512bw = avx512_os && ((r[1] & (1 << 30)) != 0);
        info->has_avx512vl = avx512_os && ((r[1] & (1u << 31)) != 0);
    }

    if (nx_hw_has_leaf(4)) {
        uint32_t l1 = 0, l2 = 0, l3 = 0;
        for (int i = 0; i < 8; ++i) {
            nx_cpuid(r, 4, i);
            uint32_t cache_type = (uint32_t)(r[0] & 0x1F);
            if (cache_type == 0) break;
            uint32_t level = (uint32_t)((r[0] >> 5) & 0x7);
            uint32_t line_size = (uint32_t)(r[1] & 0xFFF) + 1u;
            uint32_t partitions = (uint32_t)((r[1] >> 12) & 0x3FF) + 1u;
            uint32_t ways = (uint32_t)((r[1] >> 22) & 0x3FF) + 1u;
            uint32_t sets = (uint32_t)r[2] + 1u;
            uint32_t bytes = line_size * partitions * ways * sets;
            info->cache_line_bytes = line_size;
            if (level == 1 && cache_type == 1) l1 = bytes;
            else if (level == 2) l2 = bytes;
            else if (level == 3) l3 = bytes;
        }
        info->l1_data_cache_bytes = l1;
        info->l2_cache_bytes = l2;
        info->l3_cache_bytes = l3;
    }
}
#endif

NxResult nx_hardware_query(NxHardwareInfo* info) {
    if (!info) return NX_ERROR_ARGUMENT;
    nx_hw_zero(info);

#if defined(_WIN32)
    strcpy_s(info->platform, sizeof(info->platform), "Windows x64");
    SYSTEM_INFO sys_info;
    GetNativeSystemInfo(&sys_info);
    info->logical_processors = sys_info.dwNumberOfProcessors;
    info->physical_cores = info->logical_processors;

    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(mem_status);
    if (GlobalMemoryStatusEx(&mem_status)) {
        info->total_memory_bytes = mem_status.ullTotalPhys;
        info->available_memory_bytes = mem_status.ullAvailPhys;
    }
    nx_hw_query_x86(info);
#elif defined(__linux__)
    strncpy(info->platform, "Linux x64", sizeof(info->platform) - 1);
    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    info->logical_processors = cpus > 0 ? (uint32_t)cpus : 1u;
    info->physical_cores = info->logical_processors;
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    long avail_pages = sysconf(_SC_AVPHYS_PAGES);
    if (pages > 0 && page_size > 0) info->total_memory_bytes = (uint64_t)pages * (uint64_t)page_size;
    if (avail_pages > 0 && page_size > 0) info->available_memory_bytes = (uint64_t)avail_pages * (uint64_t)page_size;
#if defined(__x86_64__) || defined(__i386__)
    nx_hw_query_x86(info);
#endif
#elif defined(__APPLE__)
    strncpy(info->platform, "macOS", sizeof(info->platform) - 1);
    long cpus = sysconf(_SC_NPROCESSORS_ONLN);
    info->logical_processors = cpus > 0 ? (uint32_t)cpus : 1u;
    info->physical_cores = info->logical_processors;
#if defined(__x86_64__) || defined(__i386__)
    nx_hw_query_x86(info);
#else
    strncpy(info->cpu_vendor_string, "Apple", sizeof(info->cpu_vendor_string) - 1);
    info->cpu_vendor = NX_HARDWARE_VENDOR_APPLE;
#endif
#else
    strncpy(info->platform, "Generic", sizeof(info->platform) - 1);
    info->logical_processors = 1;
    info->physical_cores = 1;
#endif

    if (info->physical_cores == 0) info->physical_cores = info->logical_processors;
    return NX_OK;
}

void nx_hardware_print(const NxHardwareInfo* info) {
    if (!info) return;
    printf("Nexiora Hardware\n");
    printf("  Platform: %s\n", info->platform);
    printf("  CPU vendor: %s (%s)\n", nx_hardware_vendor_to_string(info->cpu_vendor), info->cpu_vendor_string[0] ? info->cpu_vendor_string : "unknown");
    printf("  CPU brand: %s\n", info->cpu_brand_string[0] ? info->cpu_brand_string : "unknown");
    printf("  Logical processors: %u\n", info->logical_processors);
    printf("  Physical cores: %u\n", info->physical_cores);
    printf("  Total memory: %.2f GB\n", (double)info->total_memory_bytes / (1024.0 * 1024.0 * 1024.0));
    printf("  Available memory: %.2f GB\n", (double)info->available_memory_bytes / (1024.0 * 1024.0 * 1024.0));
    printf("  Cache line: %u bytes\n", info->cache_line_bytes);
    printf("  L1 data cache: %u KB\n", info->l1_data_cache_bytes / 1024u);
    printf("  L2 cache: %u KB\n", info->l2_cache_bytes / 1024u);
    printf("  L3 cache: %u KB\n", info->l3_cache_bytes / 1024u);
    printf("  SIMD: MMX=%s SSE=%s SSE2=%s SSE3=%s SSSE3=%s SSE4.1=%s SSE4.2=%s\n",
           info->has_mmx ? "yes" : "no", info->has_sse ? "yes" : "no", info->has_sse2 ? "yes" : "no",
           info->has_sse3 ? "yes" : "no", info->has_ssse3 ? "yes" : "no",
           info->has_sse41 ? "yes" : "no", info->has_sse42 ? "yes" : "no");
    printf("  SIMD: AVX=%s AVX2=%s AVX512F=%s AVX512DQ=%s AVX512CD=%s AVX512BW=%s AVX512VL=%s\n",
           info->has_avx ? "yes" : "no", info->has_avx2 ? "yes" : "no",
           info->has_avx512f ? "yes" : "no", info->has_avx512dq ? "yes" : "no",
           info->has_avx512cd ? "yes" : "no", info->has_avx512bw ? "yes" : "no",
           info->has_avx512vl ? "yes" : "no");
    printf("  CPU features: FMA=%s AES=%s BMI1=%s BMI2=%s POPCNT=%s\n",
           info->has_fma ? "yes" : "no", info->has_aes ? "yes" : "no",
           info->has_bmi1 ? "yes" : "no", info->has_bmi2 ? "yes" : "no",
           info->has_popcnt ? "yes" : "no");
}
