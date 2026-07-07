#include "Nexiora/NCP/Runtime/NxRuntime.h"
#include "Nexiora/NCP/Memory/NxMemory.h"
#include "Nexiora/NCP/Common/NxResult.h"
#include <stdio.h>
#include <string.h>

#define NX_TEST_ASSERT(condition) do { \
    if (!(condition)) { \
        printf("FAILED: %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int test_result_strings(void) {
    NX_TEST_ASSERT(strcmp(nx_result_to_string(NX_OK), "NX_OK") == 0);
    NX_TEST_ASSERT(strcmp(nx_result_to_string(NX_ERROR_MEMORY), "NX_ERROR_MEMORY") == 0);
    return 0;
}

static int test_memory_stats(void) {
    NX_TEST_ASSERT(nx_memory_initialize() == NX_OK);
    NxMemoryStats before = nx_memory_get_stats();
    void* block = nx_memory_allocate(128);
    NX_TEST_ASSERT(block != NULL);
    NxMemoryStats after_alloc = nx_memory_get_stats();
    NX_TEST_ASSERT(after_alloc.allocation_count == before.allocation_count + 1);
    NX_TEST_ASSERT(after_alloc.bytes_current >= before.bytes_current + 128);
    nx_memory_free(block);
    NxMemoryStats after_free = nx_memory_get_stats();
    NX_TEST_ASSERT(after_free.free_count == after_alloc.free_count + 1);
    NX_TEST_ASSERT(after_free.bytes_current == before.bytes_current);
    nx_memory_shutdown();
    return 0;
}

static int test_runtime_lifecycle(void) {
    NxRuntime runtime;
    NX_TEST_ASSERT(nx_runtime_initialize(&runtime, "nexiora-test.log") == NX_OK);
    NX_TEST_ASSERT(runtime.initialized == 1);
    NX_TEST_ASSERT(runtime.hardware.logical_processors >= 1);
    nx_runtime_shutdown(&runtime);
    NX_TEST_ASSERT(runtime.initialized == 0);
    return 0;
}

int main(void) {
    int failed = 0;
    failed += test_result_strings();
    failed += test_memory_stats();
    failed += test_runtime_lifecycle();

    if (failed == 0) {
        printf("Nexiora Core Tests: PASS\n");
        return 0;
    }

    printf("Nexiora Core Tests: FAIL\n");
    return 1;
}
