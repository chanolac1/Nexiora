#include "Nexiora/Research/NxPersistentMemory.h"

#include <stdio.h>
#include <string.h>

static int Expect(int condition, const char* message)
{
    if (!condition) { printf("FAIL: %s\n", message); return 1; }
    return 0;
}

static int TestMemoryStoresTypedRecords(void)
{
    NxPersistentMemory memory;
    NxPersistentMemorySummary summary;
    uint32_t id = 0;
    const NxMemoryRecord* record;
    if (Expect(NxPersistentMemory_Init(&memory) == NX_PERSISTENT_MEMORY_OK, "init should succeed")) return 1;
    if (Expect(NxPersistentMemory_AddRecord(&memory, NX_MEMORY_RECORD_FACT, "SIMD", "SIMD improves vector benchmarks", "test", "2026-07-08", 91, &id) == NX_PERSISTENT_MEMORY_OK, "add fact should succeed")) return 1;
    if (Expect(NxPersistentMemory_AddRecord(&memory, NX_MEMORY_RECORD_DECISION, "Human gate", "Runtime promotion requires human approval", "BOOK", "2026-07-08", 100, &id) == NX_PERSISTENT_MEMORY_OK, "add decision should succeed")) return 1;
    if (Expect(NxPersistentMemory_GetRecordCount(&memory) == 2, "record count should be 2")) return 1;
    if (Expect(NxPersistentMemory_CountByType(&memory, NX_MEMORY_RECORD_FACT) == 1, "fact count should be 1")) return 1;
    record = NxPersistentMemory_FindByTitle(&memory, "Human gate");
    if (Expect(record != 0, "decision should be found by title")) return 1;
    if (Expect(record->confidence == 100, "confidence should match")) return 1;
    if (Expect(NxPersistentMemory_GetSummary(&memory, &summary) == NX_PERSISTENT_MEMORY_OK, "summary should succeed")) return 1;
    if (Expect(summary.facts == 1 && summary.decisions == 1, "summary counts should match")) return 1;
    NxPersistentMemory_Shutdown(&memory);
    return 0;
}

static int TestMemoryValidatesConfidence(void)
{
    NxPersistentMemory memory;
    uint32_t id = 0;
    if (Expect(NxPersistentMemory_Init(&memory) == NX_PERSISTENT_MEMORY_OK, "init should succeed")) return 1;
    if (Expect(NxPersistentMemory_AddRecord(&memory, NX_MEMORY_RECORD_FACT, "Bad", "Bad", "test", "2026-07-08", 101, &id) == NX_PERSISTENT_MEMORY_INVALID_CONFIDENCE, "invalid confidence should fail")) return 1;
    NxPersistentMemory_Shutdown(&memory);
    return 0;
}

static int TestMemorySaveAndLoad(void)
{
    NxPersistentMemory memory;
    NxPersistentMemory loaded;
    uint32_t id = 0;
    const char* path = "persistent_memory_test.jsonl";
    if (Expect(NxPersistentMemory_Init(&memory) == NX_PERSISTENT_MEMORY_OK, "init memory should succeed")) return 1;
    if (Expect(NxPersistentMemory_Init(&loaded) == NX_PERSISTENT_MEMORY_OK, "init loaded should succeed")) return 1;
    if (Expect(NxPersistentMemory_AddRecord(&memory, NX_MEMORY_RECORD_CONCEPT, "Scheduler", "Research scheduler chooses work", "test", "2026-07-08", 88, &id) == NX_PERSISTENT_MEMORY_OK, "add concept should succeed")) return 1;
    if (Expect(NxPersistentMemory_SaveJsonl(&memory, path) == NX_PERSISTENT_MEMORY_OK, "save should succeed")) return 1;
    if (Expect(NxPersistentMemory_LoadJsonl(&loaded, path) == NX_PERSISTENT_MEMORY_OK, "load should succeed")) return 1;
    if (Expect(NxPersistentMemory_GetRecordCount(&loaded) == 1, "loaded count should be 1")) return 1;
    if (Expect(NxPersistentMemory_FindByTitle(&loaded, "Scheduler") != 0, "loaded record should be found")) return 1;
    remove(path);
    NxPersistentMemory_Shutdown(&loaded);
    NxPersistentMemory_Shutdown(&memory);
    return 0;
}

int main(void)
{
    if (TestMemoryStoresTypedRecords() != 0) return 1;
    if (TestMemoryValidatesConfidence() != 0) return 1;
    if (TestMemorySaveAndLoad() != 0) return 1;
    printf("NxPersistentMemoryTests passed\n");
    return 0;
}
