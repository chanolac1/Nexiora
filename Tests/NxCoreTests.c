#include "Nexiora/NCP/Runtime/NxRuntime.h"
#include "Nexiora/NCP/Memory/NxMemory.h"
#include "Nexiora/NCP/Common/NxResult.h"
#include "Nexiora/NCP/Hardware/NxHardware.h"
#include "Nexiora/NCP/String/NxString.h"
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
    NX_TEST_ASSERT(nx_memory_is_aligned(block, NX_MEMORY_DEFAULT_ALIGNMENT));
    NxMemoryStats after_alloc = nx_memory_get_stats();
    NX_TEST_ASSERT(after_alloc.allocation_count == before.allocation_count + 1);
    NX_TEST_ASSERT(after_alloc.bytes_current >= before.bytes_current + 128);
    NX_TEST_ASSERT(after_alloc.bytes_total_allocated >= before.bytes_total_allocated + 128);
    nx_memory_free(block);
    NxMemoryStats after_free = nx_memory_get_stats();
    NX_TEST_ASSERT(after_free.free_count == after_alloc.free_count + 1);
    NX_TEST_ASSERT(after_free.bytes_current == before.bytes_current);
    nx_memory_shutdown();
    return 0;
}

static int test_memory_aligned_allocation(void) {
    NX_TEST_ASSERT(nx_memory_initialize() == NX_OK);
    void* p16 = nx_memory_allocate_aligned(33, 16);
    void* p64 = nx_memory_allocate_aligned(33, 64);
    void* p256 = nx_memory_allocate_aligned(33, 256);
    NX_TEST_ASSERT(p16 != NULL);
    NX_TEST_ASSERT(p64 != NULL);
    NX_TEST_ASSERT(p256 != NULL);
    NX_TEST_ASSERT(nx_memory_is_aligned(p16, 16));
    NX_TEST_ASSERT(nx_memory_is_aligned(p64, 64));
    NX_TEST_ASSERT(nx_memory_is_aligned(p256, 256));
    nx_memory_free(p16);
    nx_memory_free(p64);
    nx_memory_free(p256);
    NxMemoryStats stats = nx_memory_get_stats();
    NX_TEST_ASSERT(stats.bytes_current == 0);
    nx_memory_shutdown();
    return 0;
}

static int test_memory_zeroed(void) {
    NX_TEST_ASSERT(nx_memory_initialize() == NX_OK);
    unsigned char* data = (unsigned char*)nx_memory_allocate_zeroed(32, sizeof(unsigned char));
    NX_TEST_ASSERT(data != NULL);
    for (int i = 0; i < 32; ++i) {
        NX_TEST_ASSERT(data[i] == 0);
    }
    nx_memory_free(data);
    nx_memory_shutdown();
    return 0;
}

static int test_arena_lifecycle(void) {
    NX_TEST_ASSERT(nx_memory_initialize() == NX_OK);
    NxArena arena;
    NX_TEST_ASSERT(nx_arena_create(&arena, 4096, 64) == NX_OK);
    void* a = nx_arena_allocate(&arena, 128);
    void* b = nx_arena_allocate_zeroed(&arena, 256);
    NX_TEST_ASSERT(a != NULL);
    NX_TEST_ASSERT(b != NULL);
    NX_TEST_ASSERT(nx_memory_is_aligned(a, 64));
    NX_TEST_ASSERT(nx_memory_is_aligned(b, 64));
    NxArenaStats stats = nx_arena_get_stats(&arena);
    NX_TEST_ASSERT(stats.used_bytes >= 384);
    NX_TEST_ASSERT(stats.peak_bytes >= stats.used_bytes);
    nx_arena_reset(&arena);
    stats = nx_arena_get_stats(&arena);
    NX_TEST_ASSERT(stats.used_bytes == 0);
    NX_TEST_ASSERT(stats.peak_bytes >= 384);
    nx_arena_destroy(&arena);
    NxMemoryStats memory_stats = nx_memory_get_stats();
    NX_TEST_ASSERT(memory_stats.bytes_current == 0);
    nx_memory_shutdown();
    return 0;
}


static int test_hardware_query(void) {
    NxHardwareInfo info;
    NX_TEST_ASSERT(nx_hardware_query(&info) == NX_OK);
    NX_TEST_ASSERT(info.logical_processors >= 1);
    NX_TEST_ASSERT(info.cache_line_bytes >= 16);
    NX_TEST_ASSERT(info.platform[0] != '\0');
    NX_TEST_ASSERT(nx_hardware_vendor_to_string(info.cpu_vendor) != NULL);
    return 0;
}


static int test_string_primitives(void) {
    char buffer[32];
    NX_TEST_ASSERT(nx_string_length("Nexiora") == 7);
    NX_TEST_ASSERT(nx_string_length(NULL) == 0);
    NX_TEST_ASSERT(nx_string_length_bounded("Nexiora", 3) == 3);
    NX_TEST_ASSERT(nx_string_compare("abc", "abc") == 0);
    NX_TEST_ASSERT(nx_string_compare("abc", "abd") < 0);
    NX_TEST_ASSERT(nx_string_compare("abd", "abc") > 0);
    NX_TEST_ASSERT(nx_string_compare_n("abcdef", "abcXYZ", 3) == 0);
    NX_TEST_ASSERT(nx_string_copy(buffer, sizeof(buffer), "Genesis") == NX_OK);
    NX_TEST_ASSERT(strcmp(buffer, "Genesis") == 0);
    NX_TEST_ASSERT(nx_string_copy(buffer, 4, "Genesis") == NX_ERROR_ARGUMENT);
    NX_TEST_ASSERT(strcmp(buffer, "Gen") == 0);
    NX_TEST_ASSERT(nx_string_find_char("Nexiora", 'i') != NULL);
    NX_TEST_ASSERT(nx_string_find_char("Nexiora", 'z') == NULL);
    NxStringView a = nx_string_view_from_cstr("NCP");
    NxStringView b = nx_string_view_from_cstr("NCP");
    NxStringView c = nx_string_view_from_cstr("LLM");
    NX_TEST_ASSERT(nx_string_view_equals(a, b) == 1);
    NX_TEST_ASSERT(nx_string_view_equals(a, c) == 0);
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
    failed += test_memory_aligned_allocation();
    failed += test_memory_zeroed();
    failed += test_arena_lifecycle();
    failed += test_hardware_query();
    failed += test_string_primitives();
    failed += test_runtime_lifecycle();

    if (failed == 0) {
        printf("Nexiora Core Tests: PASS\n");
        return 0;
    }

    printf("Nexiora Core Tests: FAIL\n");
    return 1;
}
