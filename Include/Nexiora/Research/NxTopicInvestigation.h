#ifndef NEXIORA_RESEARCH_NX_TOPIC_INVESTIGATION_H
#define NEXIORA_RESEARCH_NX_TOPIC_INVESTIGATION_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxTopicInvestigationStatus
{
    NX_TOPIC_INVESTIGATION_OK = 0,
    NX_TOPIC_INVESTIGATION_INVALID_ARGUMENT = 1,
    NX_TOPIC_INVESTIGATION_IO_ERROR = 2
} NxTopicInvestigationStatus;

typedef struct NxTopicInvestigationResult
{
    char topic[128];
    char output_dir[512];
    char report_path[512];
    char memory_path[512];
    unsigned source_count;
    unsigned concept_count;
    unsigned relation_count;
    unsigned contradiction_count;
    unsigned confidence_percent;
} NxTopicInvestigationResult;

const char* NxTopicInvestigation_StatusToString(NxTopicInvestigationStatus status);

NxTopicInvestigationStatus NxTopicInvestigation_Run(
    const char* root_path,
    const char* topic,
    FILE* output,
    NxTopicInvestigationResult* result_out);

#ifdef __cplusplus
}
#endif

#endif
