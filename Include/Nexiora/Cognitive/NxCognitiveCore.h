#ifndef NEXIORA_COGNITIVE_NX_COGNITIVE_CORE_H
#define NEXIORA_COGNITIVE_NX_COGNITIVE_CORE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxCognitiveStatus
{
    NX_COGNITIVE_OK = 0,
    NX_COGNITIVE_INVALID_ARGUMENT = 1,
    NX_COGNITIVE_IO_ERROR = 2,
    NX_COGNITIVE_UNSUPPORTED_INPUT = 3,
    NX_COGNITIVE_NOT_FOUND = 4,
    NX_COGNITIVE_BUFFER_TOO_SMALL = 5
} NxCognitiveStatus;

typedef struct NxCognitiveIngestResult
{
    char topic[128];
    char topic_dir[512];
    char source_path[512];
    char memory_path[512];
    char concepts_path[512];
    char chunks_path[512];
    int source_is_textual;
    size_t bytes_read;
    size_t chunks_written;
    size_t concepts_written;
} NxCognitiveIngestResult;

typedef struct NxCognitiveAnswer
{
    char topic[128];
    char question[512];
    char answer[4096];
    char answer_path[512];
    int confidence;
    size_t matched_chunks;
} NxCognitiveAnswer;

const char* NxCognitive_StatusToString(NxCognitiveStatus status);

NxCognitiveStatus NxCognitive_NormalizeTopic(
    const char* topic,
    char* output,
    size_t output_size);

NxCognitiveStatus NxCognitive_IngestFile(
    const char* workspace_root,
    const char* topic,
    const char* input_path,
    NxCognitiveIngestResult* result_out);

NxCognitiveStatus NxCognitive_Ask(
    const char* workspace_root,
    const char* topic,
    const char* question,
    NxCognitiveAnswer* answer_out);

NxCognitiveStatus NxCognitive_Inspect(
    const char* workspace_root,
    const char* topic,
    char* output,
    size_t output_size);

#ifdef __cplusplus
}
#endif

#endif
