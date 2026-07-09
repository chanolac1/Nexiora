#ifndef NEXIORA_RESEARCH_NX_KNOWLEDGE_ACQUISITION_H
#define NEXIORA_RESEARCH_NX_KNOWLEDGE_ACQUISITION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_KA_MAX_TOPIC_LENGTH 128
#define NX_KA_MAX_KIND_LENGTH 32
#define NX_KA_MAX_SOURCE_NAME_LENGTH 96
#define NX_KA_MAX_SOURCE_REASON_LENGTH 192
#define NX_KA_MAX_STEP_LENGTH 160
#define NX_KA_MAX_SOURCES 12
#define NX_KA_MAX_STEPS 12
#define NX_KA_MAX_PATH_LENGTH 260

typedef enum NxKnowledgeAcquisitionStatus
{
    NX_KA_STATUS_OK = 0,
    NX_KA_STATUS_INVALID_ARGUMENT = 1,
    NX_KA_STATUS_TOPIC_TOO_LONG = 2,
    NX_KA_STATUS_IO_ERROR = 3
} NxKnowledgeAcquisitionStatus;

typedef enum NxKnowledgeSourceType
{
    NX_KNOWLEDGE_SOURCE_OFFICIAL_DOCUMENTATION = 0,
    NX_KNOWLEDGE_SOURCE_GITHUB_REPOSITORY = 1,
    NX_KNOWLEDGE_SOURCE_RFC = 2,
    NX_KNOWLEDGE_SOURCE_RESEARCH_PAPER = 3,
    NX_KNOWLEDGE_SOURCE_BOOK = 4,
    NX_KNOWLEDGE_SOURCE_LOCAL_FILE = 5,
    NX_KNOWLEDGE_SOURCE_TECHNICAL_ARTICLE = 6,
    NX_KNOWLEDGE_SOURCE_UNKNOWN = 7
} NxKnowledgeSourceType;

typedef struct NxKnowledgeSourceCandidate
{
    NxKnowledgeSourceType type;
    char name[NX_KA_MAX_SOURCE_NAME_LENGTH];
    char reason[NX_KA_MAX_SOURCE_REASON_LENGTH];
    unsigned int trust_score;
    unsigned int priority;
} NxKnowledgeSourceCandidate;

typedef struct NxKnowledgeAcquisitionPlan
{
    char topic[NX_KA_MAX_TOPIC_LENGTH];
    char kind[NX_KA_MAX_KIND_LENGTH];
    NxKnowledgeSourceCandidate sources[NX_KA_MAX_SOURCES];
    size_t source_count;
    char steps[NX_KA_MAX_STEPS][NX_KA_MAX_STEP_LENGTH];
    size_t step_count;
    unsigned int estimated_minutes;
    unsigned int expected_confidence;
} NxKnowledgeAcquisitionPlan;

const char* NxKnowledgeAcquisition_StatusToString(NxKnowledgeAcquisitionStatus status);
const char* NxKnowledgeAcquisition_SourceTypeToString(NxKnowledgeSourceType type);

NxKnowledgeAcquisitionStatus NxKnowledgeAcquisition_BuildPlan(
    const char* topic,
    NxKnowledgeAcquisitionPlan* plan_out);

NxKnowledgeAcquisitionStatus NxKnowledgeAcquisition_WritePlanMarkdown(
    const NxKnowledgeAcquisitionPlan* plan,
    const char* root_path,
    char* output_path,
    size_t output_path_size);

NxKnowledgeAcquisitionStatus NxKnowledgeAcquisition_WritePlanJson(
    const NxKnowledgeAcquisitionPlan* plan,
    const char* root_path,
    char* output_path,
    size_t output_path_size);

#ifdef __cplusplus
}
#endif

#endif
