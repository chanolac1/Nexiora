#ifndef NEXIORA_RESEARCH_NX_PERSISTENT_MEMORY_H
#define NEXIORA_RESEARCH_NX_PERSISTENT_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxPersistentMemoryStatus
{
    NX_PERSISTENT_MEMORY_OK = 0,
    NX_PERSISTENT_MEMORY_INVALID_ARGUMENT = 1,
    NX_PERSISTENT_MEMORY_OUT_OF_MEMORY = 2,
    NX_PERSISTENT_MEMORY_IO_ERROR = 3,
    NX_PERSISTENT_MEMORY_NOT_FOUND = 4,
    NX_PERSISTENT_MEMORY_INVALID_CONFIDENCE = 5
} NxPersistentMemoryStatus;

typedef enum NxMemoryRecordType
{
    NX_MEMORY_RECORD_FACT = 1,
    NX_MEMORY_RECORD_DECISION = 2,
    NX_MEMORY_RECORD_HYPOTHESIS = 3,
    NX_MEMORY_RECORD_CONCEPT = 4
} NxMemoryRecordType;

typedef struct NxMemoryRecord
{
    uint32_t id;
    NxMemoryRecordType type;
    char title[96];
    char body[256];
    char source[128];
    char timestamp[32];
    int confidence;
} NxMemoryRecord;

typedef struct NxPersistentMemory
{
    NxMemoryRecord* records;
    size_t record_count;
    size_t record_capacity;
    uint32_t next_id;
} NxPersistentMemory;

typedef struct NxPersistentMemorySummary
{
    size_t facts;
    size_t decisions;
    size_t hypotheses;
    size_t concepts;
    int average_confidence;
} NxPersistentMemorySummary;

NxPersistentMemoryStatus NxPersistentMemory_Init(NxPersistentMemory* memory);
void NxPersistentMemory_Shutdown(NxPersistentMemory* memory);
void NxPersistentMemory_Clear(NxPersistentMemory* memory);

NxPersistentMemoryStatus NxPersistentMemory_AddRecord(
    NxPersistentMemory* memory,
    NxMemoryRecordType type,
    const char* title,
    const char* body,
    const char* source,
    const char* timestamp,
    int confidence,
    uint32_t* record_id_out);

size_t NxPersistentMemory_GetRecordCount(const NxPersistentMemory* memory);
size_t NxPersistentMemory_CountByType(const NxPersistentMemory* memory, NxMemoryRecordType type);
const NxMemoryRecord* NxPersistentMemory_FindById(const NxPersistentMemory* memory, uint32_t id);
const NxMemoryRecord* NxPersistentMemory_FindByTitle(const NxPersistentMemory* memory, const char* title);

NxPersistentMemoryStatus NxPersistentMemory_GetSummary(
    const NxPersistentMemory* memory,
    NxPersistentMemorySummary* summary_out);

NxPersistentMemoryStatus NxPersistentMemory_SaveJsonl(
    const NxPersistentMemory* memory,
    const char* path);

NxPersistentMemoryStatus NxPersistentMemory_LoadJsonl(
    NxPersistentMemory* memory,
    const char* path);

NxPersistentMemoryStatus NxPersistentMemory_SeedFromFirstAutonomousExecution(
    const char* repository_root,
    char* memory_path_out,
    size_t memory_path_out_size);

const char* NxPersistentMemory_RecordTypeToString(NxMemoryRecordType type);
const char* NxPersistentMemory_StatusToString(NxPersistentMemoryStatus status);

#ifdef __cplusplus
}
#endif

#endif
