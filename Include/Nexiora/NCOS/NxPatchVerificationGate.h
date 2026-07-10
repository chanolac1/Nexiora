#ifndef NEXIORA_NCOS_NX_PATCH_VERIFICATION_GATE_H
#define NEXIORA_NCOS_NX_PATCH_VERIFICATION_GATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxPatchVerificationResult {
    char id[128];
    char status[32];
    char application_record[512];
    char verification_report[512];
    int approval_found;
    int application_found;
    int tests_passed;
    int warnings;
    int errors;
    int promotable;
} NxPatchVerificationResult;

int NxPatchVerificationGate_Verify(const char* root, const char* id, NxPatchVerificationResult* out);
int NxPatchVerificationGate_Status(const char* root, const char* id, NxPatchVerificationResult* out);

#ifdef __cplusplus
}
#endif

#endif
