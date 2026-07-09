#ifndef NEXIORA_NCOS_NX_CONCEPT_GRAPH_H
#define NEXIORA_NCOS_NX_CONCEPT_GRAPH_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxConceptGraphEdge
{
    char domain[128];
    char from[128];
    char relation[128];
    char to[128];
    char source[256];
    int confidence;
    char path[512];
} NxConceptGraphEdge;

typedef struct NxConceptGraphStats
{
    int edge_count;
    int outgoing_count;
    int incoming_count;
    char path[512];
} NxConceptGraphStats;

int NxConceptGraph_Link(
    const char* root,
    const char* domain,
    const char* from,
    const char* relation,
    const char* to,
    const char* source,
    int confidence,
    NxConceptGraphEdge* edge_out);

int NxConceptGraph_Find(
    const char* root,
    const char* domain,
    const char* from,
    const char* relation,
    const char* to,
    NxConceptGraphEdge* edge_out);

int NxConceptGraph_Stats(
    const char* root,
    const char* domain,
    const char* concept,
    NxConceptGraphStats* stats_out);

int NxConceptGraph_Show(
    const char* root,
    const char* domain,
    const char* concept,
    char* output,
    size_t output_size);

void NxConceptGraph_Normalize(const char* input, char* output, size_t output_size);

#ifdef __cplusplus
}
#endif

#endif
