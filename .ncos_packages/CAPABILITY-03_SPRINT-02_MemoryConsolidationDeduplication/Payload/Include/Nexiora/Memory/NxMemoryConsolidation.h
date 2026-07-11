#ifndef NEXIORA_MEMORY_NX_MEMORY_CONSOLIDATION_H
#define NEXIORA_MEMORY_NX_MEMORY_CONSOLIDATION_H

#include "Nexiora/Memory/NxVersionedSemanticMemory.h"

#define NX_MC_MAX_SOURCES 32U
#define NX_MC_MAX_SUMMARY 1024U

typedef enum NxMcStatus {
    NX_MC_OK = 0,
    NX_MC_UNCHANGED = 1,
    NX_MC_CONFLICT_DETECTED = 2,
    NX_MC_INVALID_ARGUMENT = 3,
    NX_MC_NOT_FOUND = 4,
    NX_MC_IO_ERROR = 5
} NxMcStatus;

typedef struct NxMcReport {
    NxMcStatus status;
    unsigned int active_entry_id;
    unsigned int active_version;
    unsigned int old_confidence;
    unsigned int proposed_confidence;
    unsigned int supporting_entries;
    unsigned int independent_sources;
    unsigned int opposing_entries;
    unsigned int duplicate_entries;
    unsigned int similarity_threshold;
    char concept[NX_VSM_MAX_CONCEPT];
    char active_belief[NX_VSM_MAX_BELIEF];
    char summary[NX_MC_MAX_SUMMARY];
} NxMcReport;

NxMcStatus NxMemoryConsolidation_Analyze(
    const char* memory_path,
    const char* concept,
    unsigned int similarity_threshold,
    NxMcReport* out_report);

NxMcStatus NxMemoryConsolidation_Consolidate(
    const char* memory_path,
    const char* concept,
    const char* timestamp,
    unsigned int similarity_threshold,
    NxMcReport* out_report);

unsigned int NxMemoryConsolidation_Similarity(const char* left, const char* right);
const char* NxMemoryConsolidation_StatusName(NxMcStatus status);

#endif
