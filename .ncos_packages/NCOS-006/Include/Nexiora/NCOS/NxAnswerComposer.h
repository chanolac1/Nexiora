#ifndef NEXIORA_NCOS_NX_ANSWER_COMPOSER_H
#define NEXIORA_NCOS_NX_ANSWER_COMPOSER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxAnswerComposerResult
{
    char domain[128];
    char source[128];
    char target[128];
    char intent[64];
    char answer[4096];
    char evidence[2048];
    int confidence;
    int evidence_count;
} NxAnswerComposerResult;

int NxAnswerComposer_ComposeRelationAnswer(const char* root_path,
                                           const char* domain,
                                           const char* source_concept,
                                           const char* target_concept,
                                           NxAnswerComposerResult* result_out);

int NxAnswerComposer_WriteAnswer(const char* path, const NxAnswerComposerResult* result);

#ifdef __cplusplus
}
#endif

#endif
