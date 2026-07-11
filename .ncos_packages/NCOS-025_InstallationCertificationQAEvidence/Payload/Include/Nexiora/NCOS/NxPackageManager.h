#ifndef NEXIORA_NCOS_PACKAGE_MANAGER_H
#define NEXIORA_NCOS_PACKAGE_MANAGER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxPackageInstallResult
{
    char package_id[128];
    char package_version[64];
    char registry_path[512];
    char install_log_path[512];
    int files_declared;
    int files_installed;
    int files_skipped;
    int files_backed_up;
    int files_rolled_back;
    int transaction_committed;
    char transaction_path[512];
    char transaction_id[32];
    char history_path[512];
    int history_entries;
    int success;
} NxPackageInstallResult;




typedef struct NxPackageApplyResult
{
    char package_id[128];
    char package_version[64];
    char transaction_id[32];
    char apply_log_path[512];
    char certification_report_path[512];
    char certification_text_path[512];
    char release_recommendation[32];
    char failed_phase[64];
    char message[256];
    int verify_passed;
    int dependencies_passed;
    int install_passed;
    int configure_passed;
    int build_passed;
    int warning_gate_passed;
    int tests_passed;
    int documentation_passed;
    int artifacts_passed;
    int qa_repeatability_passed;
    int certification_generated;
    int rolled_back;
    int success;
} NxPackageApplyResult;

typedef struct NxPackageHistoryResult
{
    char package_id[128];
    char history_path[512];
    int entries;
    int success;
} NxPackageHistoryResult;

typedef struct
{
    int success;
    int manifest_found;
    int files_declared;
    int payload_files_found;
    int payload_files_missing;
    int dependencies_declared;
    int dependencies_satisfied;
    int dependencies_missing;
    char package_id[128];
    char package_version[64];
    char message[256];
} NxPackageVerifyResult;


int NxPackageManager_Apply(const char* repo_root,
                           const char* package_dir,
                           NxPackageApplyResult* out_result);

int NxPackageManager_ApplyLogHasWarnings(const char* log_path);

int NxPackageManager_Install(const char* repo_root,
                             const char* package_dir,
                             NxPackageInstallResult* out_result);

int NxPackageManager_Rollback(const char* repo_root,
                              const char* package_id,
                              NxPackageInstallResult* out_result);

int NxPackageManager_Status(const char* repo_root,
                            const char* package_id,
                            NxPackageInstallResult* out_result);

int NxPackageManager_History(const char* repo_root,
                             const char* package_id,
                             NxPackageHistoryResult* out_result);

int NxPackageManager_RollbackTransaction(const char* repo_root,
                                         const char* package_id,
                                         const char* transaction_id,
                                         NxPackageInstallResult* out_result);

int NxPackageManager_VerifyPackage(const char* package_dir, NxPackageVerifyResult* out_result);

int NxPackageManager_VerifyDependencies(const char* repo_root,
                                        const char* package_dir,
                                        NxPackageVerifyResult* out_result);

#ifdef __cplusplus
}
#endif

#endif
