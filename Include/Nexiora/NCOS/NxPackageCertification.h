#ifndef NEXIORA_NCOS_PACKAGE_CERTIFICATION_H
#define NEXIORA_NCOS_PACKAGE_CERTIFICATION_H

#include <stddef.h>
#include <time.h>

#include "Nexiora/NCOS/NxPackageManager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxPackageCertificationSummary
{
    int tests_registered;
    int tests_executed;
    int tests_passed;
    int tests_failed;
    int tests_not_run;
    int warnings_found;
    int errors_found;
    int required_artifacts;
    int artifacts_found;
    int artifacts_missing;
    int qa_repetitions;
    int qa_repetitions_passed;
    char final_status[32];
    char release_recommendation[32];
    char nxcert_path[512];
    char text_path[512];
} NxPackageCertificationSummary;

int NxPackageCertification_ValidateArtifacts(const char* repo_root,
                                             const char* package_dir,
                                             NxPackageCertificationSummary* summary);

int NxPackageCertification_Generate(const char* repo_root,
                                    const char* package_dir,
                                    const NxPackageApplyResult* apply_result,
                                    time_t started_at,
                                    time_t finished_at,
                                    int qa_repetitions,
                                    int qa_repetitions_passed,
                                    NxPackageCertificationSummary* out_summary);

#ifdef __cplusplus
}
#endif

#endif
