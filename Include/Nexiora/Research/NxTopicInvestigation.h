#ifndef NEXIORA_RESEARCH_NX_TOPIC_INVESTIGATION_H
#define NEXIORA_RESEARCH_NX_TOPIC_INVESTIGATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdio.h>

#define NX_TOPIC_INVESTIGATION_MAX_PATH 512
#define NX_TOPIC_INVESTIGATION_MAX_MESSAGE 256

typedef enum NxTopicInvestigationStatus
{
    NX_TOPIC_INVESTIGATION_OK = 0,
    NX_TOPIC_INVESTIGATION_INVALID_ARGUMENT = 1,
    NX_TOPIC_INVESTIGATION_IO_ERROR = 2,
    NX_TOPIC_INVESTIGATION_PLAN_FAILED = 3
} NxTopicInvestigationStatus;

typedef struct NxTopicInvestigationResult
{
    char investigation_id[64];
    char topic[128];
    char output_directory[NX_TOPIC_INVESTIGATION_MAX_PATH];
    char report_path[NX_TOPIC_INVESTIGATION_MAX_PATH];
    char memory_path[NX_TOPIC_INVESTIGATION_MAX_PATH];
    size_t sources_considered;
    size_t concepts_extracted;
    size_t relations_created;
    size_t contradictions_found;
    unsigned int confidence;
    char summary[NX_TOPIC_INVESTIGATION_MAX_MESSAGE];
} NxTopicInvestigationResult;

const char* NxTopicInvestigation_StatusToString(NxTopicInvestigationStatus status);

NxTopicInvestigationStatus NxTopicInvestigation_Run(
    const char* root_path,
    const char* topic,
    FILE* progress_output,
    NxTopicInvestigationResult* result_out);

#ifdef __cplusplus
}
#endif

#endif
