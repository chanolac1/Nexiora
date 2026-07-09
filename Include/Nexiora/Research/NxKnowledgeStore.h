#ifndef NEXIORA_RESEARCH_NX_KNOWLEDGE_STORE_H
#define NEXIORA_RESEARCH_NX_KNOWLEDGE_STORE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_KS_MAX_TEXT 2048
#define NX_KS_MAX_PATH 512

typedef enum NxKnowledgeStoreStatus
{
    NX_KS_OK = 0,
    NX_KS_INVALID_ARGUMENT = 1,
    NX_KS_NOT_FOUND = 2,
    NX_KS_IO_ERROR = 3,
    NX_KS_OUTPUT_TOO_SMALL = 4
} NxKnowledgeStoreStatus;

typedef struct NxKnowledgeAnswer
{
    char topic[128];
    char summary[NX_KS_MAX_TEXT];
    char related[512];
    char evidence[512];
    unsigned confidence_percent;
} NxKnowledgeAnswer;

const char* NxKnowledgeStore_StatusToString(NxKnowledgeStoreStatus status);

NxKnowledgeStoreStatus NxKnowledgeStore_Query(
    const char* root_path,
    const char* topic,
    NxKnowledgeAnswer* answer_out);

NxKnowledgeStoreStatus NxKnowledgeStore_FormatAnswerSpanish(
    const NxKnowledgeAnswer* answer,
    char* output,
    size_t output_size);

#ifdef __cplusplus
}
#endif

#endif
