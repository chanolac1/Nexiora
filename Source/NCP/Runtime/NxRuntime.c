#include "Nexiora/NCP/Runtime/NxRuntime.h"
#include "Nexiora/NCP/Logging/NxLog.h"
#include "Nexiora/NCP/Common/NxVersion.h"
#include <stdio.h>

NxResult nx_runtime_initialize(NxRuntime* runtime, const char* log_path) {
    if (!runtime) {
        return NX_ERROR_ARGUMENT;
    }

    runtime->initialized = 0;

    NxResult result = nx_memory_initialize();
    if (result != NX_OK) {
        return result;
    }

    result = nx_log_initialize(log_path ? log_path : "nexiora.log");
    if (result != NX_OK) {
        return result;
    }

    result = nx_hardware_query(&runtime->hardware);
    if (result != NX_OK) {
        nx_log_write(NX_LOG_ERROR, "Runtime", "Hardware query failed.");
        return result;
    }

    runtime->memory_at_start = nx_memory_get_stats();
    runtime->initialized = 1;
    nx_log_write(NX_LOG_INFO, "Runtime", "Nexiora runtime initialized.");
    return NX_OK;
}

void nx_runtime_shutdown(NxRuntime* runtime) {
    if (!runtime || !runtime->initialized) {
        return;
    }

    NxMemoryStats stats = nx_memory_get_stats();
    char buffer[256];
#if defined(_WIN32)
    sprintf_s(buffer, sizeof(buffer), "Memory current=%zu peak=%zu allocations=%llu frees=%llu",
              stats.bytes_current, stats.bytes_peak,
              (unsigned long long)stats.allocation_count,
              (unsigned long long)stats.free_count);
#else
    snprintf(buffer, sizeof(buffer), "Memory current=%zu peak=%zu allocations=%llu frees=%llu",
             stats.bytes_current, stats.bytes_peak,
             (unsigned long long)stats.allocation_count,
             (unsigned long long)stats.free_count);
#endif
    nx_log_write(NX_LOG_INFO, "Runtime", buffer);
    nx_log_write(NX_LOG_INFO, "Runtime", "Nexiora runtime shutdown.");
    nx_log_shutdown();
    nx_memory_shutdown();
    runtime->initialized = 0;
}

void nx_runtime_print_status(const NxRuntime* runtime) {
    if (!runtime || !runtime->initialized) {
        printf("Nexiora Runtime: not initialized\n");
        return;
    }

    printf("Nexiora %d.%d.%d %s\n", NX_VERSION_MAJOR, NX_VERSION_MINOR, NX_VERSION_PATCH, NX_VERSION_NAME);
    printf("Runtime: initialized\n");
    nx_hardware_print(&runtime->hardware);

    NxMemoryStats stats = nx_memory_get_stats();
    printf("Memory\n");
    printf("  Current bytes: %zu\n", stats.bytes_current);
    printf("  Peak bytes: %zu\n", stats.bytes_peak);
    printf("  Allocations: %llu\n", (unsigned long long)stats.allocation_count);
    printf("  Frees: %llu\n", (unsigned long long)stats.free_count);
}
