#ifndef NEXIORA_NCOS_NX_CONCEPT_REGISTRY_H
#define NEXIORA_NCOS_NX_CONCEPT_REGISTRY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxConceptCard
{
    char domain[96];
    char name[128];
    char normalized_name[128];
    char definition[1024];
    char purpose[1024];
    char relationships[1024];
    double confidence;
    int version;
    char path[512];
} NxConceptCard;

int NxConcept_Normalize(const char* input, char* output, size_t output_size);
int NxConcept_Upsert(const char* root_path,
                     const char* domain,
                     const char* concept_name,
                     const char* definition,
                     const char* purpose,
                     const char* relationships,
                     double confidence,
                     NxConceptCard* out_card);
int NxConcept_Load(const char* root_path,
                   const char* domain,
                   const char* concept_name,
                   NxConceptCard* out_card);
int NxConcept_FormatAnswer(const NxConceptCard* card, char* output, size_t output_size);
int NxConcept_PathFor(const char* root_path,
                      const char* domain,
                      const char* concept_name,
                      char* output,
                      size_t output_size);

#ifdef __cplusplus
}
#endif

#endif
