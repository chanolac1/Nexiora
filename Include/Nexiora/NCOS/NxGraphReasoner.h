#ifndef NEXIORA_NCOS_NX_GRAPH_REASONER_H
#define NEXIORA_NCOS_NX_GRAPH_REASONER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxGraphReasoningResult
{
    char domain[128];
    char from[128];
    char to[128];
    char answer[2048];
    char evidence[2048];
    int found_direct;
    int found_indirect;
    int evidence_count;
    int confidence;
} NxGraphReasoningResult;

int NxGraphReasoner_Explain(
    const char* root,
    const char* domain,
    const char* from,
    const char* to,
    NxGraphReasoningResult* result_out);

int NxGraphReasoner_WhyRelated(
    const char* root,
    const char* domain,
    const char* from,
    const char* to,
    char* output,
    size_t output_size);

#ifdef __cplusplus
}
#endif

#endif
