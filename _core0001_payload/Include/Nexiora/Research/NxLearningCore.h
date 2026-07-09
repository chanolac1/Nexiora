#ifndef NEXIORA_RESEARCH_NX_LEARNING_CORE_H
#define NEXIORA_RESEARCH_NX_LEARNING_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxLearningCoreStatus
{
    NX_LEARNING_CORE_OK = 0,
    NX_LEARNING_CORE_INVALID_ARGUMENT = 1,
    NX_LEARNING_CORE_SCRIPT_FAILED = 2,
    NX_LEARNING_CORE_NOT_FOUND = 3,
    NX_LEARNING_CORE_IO_FAILED = 4
} NxLearningCoreStatus;

typedef struct NxLearningCoreResult
{
    char topic[128];
    char topic_slug[128];
    char output_dir[512];
    char report_path[512];
    char memory_path[512];
    int facts_written;
    int sources_seen;
    int confidence;
} NxLearningCoreResult;

NxLearningCoreStatus NxLearningCore_Learn(const char* topic, NxLearningCoreResult* result_out);
NxLearningCoreStatus NxLearningCore_Query(const char* topic, char* buffer, unsigned long buffer_size);
const char* NxLearningCore_StatusToString(NxLearningCoreStatus status);

#ifdef __cplusplus
}
#endif

#endif
