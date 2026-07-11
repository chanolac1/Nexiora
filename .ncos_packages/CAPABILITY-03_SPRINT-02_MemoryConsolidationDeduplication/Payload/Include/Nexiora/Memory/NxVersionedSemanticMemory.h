#ifndef NEXIORA_MEMORY_NX_VERSIONED_SEMANTIC_MEMORY_H
#define NEXIORA_MEMORY_NX_VERSIONED_SEMANTIC_MEMORY_H

#include <stddef.h>

#define NX_VSM_MAX_PATH 1024U
#define NX_VSM_MAX_CONCEPT 128U
#define NX_VSM_MAX_BELIEF 1024U
#define NX_VSM_MAX_SOURCE 512U
#define NX_VSM_MAX_TIME 64U
#define NX_VSM_MAX_REASON 512U
#define NX_VSM_MAX_ENTRIES 256U

typedef enum NxVsmStatus {
    NX_VSM_OK = 0,
    NX_VSM_UNCHANGED = 1,
    NX_VSM_INVALID_ARGUMENT = 2,
    NX_VSM_IO_ERROR = 3,
    NX_VSM_NOT_FOUND = 4,
    NX_VSM_CAPACITY_EXCEEDED = 5,
    NX_VSM_INVALID_FORMAT = 6
} NxVsmStatus;

typedef struct NxVsmEntry {
    unsigned int id;
    unsigned int version;
    unsigned int supersedes_id;
    unsigned int confidence;
    int active;
    char concept[NX_VSM_MAX_CONCEPT];
    char belief[NX_VSM_MAX_BELIEF];
    char source[NX_VSM_MAX_SOURCE];
    char timestamp[NX_VSM_MAX_TIME];
    char change_reason[NX_VSM_MAX_REASON];
} NxVsmEntry;

typedef struct NxVsmStore {
    NxVsmEntry entries[NX_VSM_MAX_ENTRIES];
    size_t entry_count;
    unsigned int next_id;
} NxVsmStore;

typedef struct NxVsmRememberResult {
    NxVsmStatus status;
    unsigned int entry_id;
    unsigned int version;
    unsigned int previous_entry_id;
    unsigned int confidence;
    int belief_changed;
    char concept[NX_VSM_MAX_CONCEPT];
    char previous_belief[NX_VSM_MAX_BELIEF];
    char current_belief[NX_VSM_MAX_BELIEF];
    char explanation[NX_VSM_MAX_REASON];
} NxVsmRememberResult;

NxVsmStatus NxVersionedSemanticMemory_Init(const char* path);
NxVsmStatus NxVersionedSemanticMemory_Load(const char* path, NxVsmStore* out_store);
NxVsmStatus NxVersionedSemanticMemory_Remember(
    const char* path,
    const char* concept,
    const char* belief,
    const char* source,
    const char* timestamp,
    unsigned int confidence,
    NxVsmRememberResult* out_result);
NxVsmStatus NxVersionedSemanticMemory_Recall(
    const char* path,
    const char* concept,
    NxVsmEntry* out_entry);
size_t NxVersionedSemanticMemory_History(
    const char* path,
    const char* concept,
    NxVsmEntry* out_entries,
    size_t capacity);
const char* NxVersionedSemanticMemory_StatusName(NxVsmStatus status);

#endif
