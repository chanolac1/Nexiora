#ifndef NEXIORA_NCOS_DOCUMENTATION_MANAGER_H
#define NEXIORA_NCOS_DOCUMENTATION_MANAGER_H

#include <stddef.h>

typedef struct NxDocumentationValidationResult {
    int success;
    int files_checked;
    int files_missing;
    int structural_errors;
    int code_references_checked;
    int code_references_missing;
    char message[512];
} NxDocumentationValidationResult;

typedef struct NxDocumentationRelease {
    char sprint_id[64];
    char sprint_name[128];
    char completion_date[16];
    char next_sprint_id[64];
    char next_sprint_name[128];
    char capabilities[1024];
    char decision_id[64];
    char decision_title[128];
    char decision_body[1024];
} NxDocumentationRelease;

int NxDocumentationManager_Validate(const char* repo_root,
                                    NxDocumentationValidationResult* out_result);
int NxDocumentationManager_LoadRelease(const char* release_path,
                                       NxDocumentationRelease* out_release,
                                       char* error_message,
                                       size_t error_message_size);
int NxDocumentationManager_FinalizeSprint(const char* repo_root,
                                          const NxDocumentationRelease* release,
                                          char* error_message,
                                          size_t error_message_size);

#endif
