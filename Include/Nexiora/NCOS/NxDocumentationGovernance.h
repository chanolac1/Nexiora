#ifndef NEXIORA_NCOS_DOCUMENTATION_GOVERNANCE_H
#define NEXIORA_NCOS_DOCUMENTATION_GOVERNANCE_H

#include <stddef.h>

typedef struct NxDocumentationValidationResult {
    int success;
    int files_checked;
    int files_missing;
    int structural_errors;
    char message[256];
} NxDocumentationValidationResult;

int NxDocumentation_Validate(const char* repo_root, NxDocumentationValidationResult* out_result);
int NxDocumentation_CompleteSprint(const char* repo_root, const char* sprint, const char* capability, const char* date);
int NxDocumentation_RecordDecision(const char* repo_root, const char* decision_id, const char* title, const char* body, const char* date);

#endif
