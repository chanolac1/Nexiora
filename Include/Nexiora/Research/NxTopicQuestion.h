#ifndef NEXIORA_RESEARCH_NX_TOPIC_QUESTION_H
#define NEXIORA_RESEARCH_NX_TOPIC_QUESTION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxTopicQuestionStatus
{
    NX_TOPIC_QUESTION_OK = 0,
    NX_TOPIC_QUESTION_INVALID_ARGUMENT = 1,
    NX_TOPIC_QUESTION_NOT_FOUND = 2,
    NX_TOPIC_QUESTION_IO_FAILED = 3
} NxTopicQuestionStatus;

typedef struct NxTopicQuestionResult
{
    char topic[128];
    char topic_slug[128];
    char question[512];
    char answer_path[512];
    int evidence_items;
    int concepts_used;
    int sources_used;
    int confidence;
} NxTopicQuestionResult;

NxTopicQuestionStatus NxTopicQuestion_Ask(
    const char* topic,
    const char* question,
    char* answer_out,
    unsigned long answer_out_size,
    NxTopicQuestionResult* result_out);

const char* NxTopicQuestion_StatusToString(NxTopicQuestionStatus status);

#ifdef __cplusplus
}
#endif

#endif
