#include "Nexiora/NCOS/NxPackageCertification.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_CERT_MKDIR(path) _mkdir(path)
#define NX_CERT_SEP '\\'
#else
#define NX_CERT_MKDIR(path) mkdir((path), 0777)
#define NX_CERT_SEP '/'
#endif

#define NX_CERT_PATH_MAX 1024
#define NX_CERT_LINE_MAX 2048

static int nx_cert_exists(const char* path)
{
    struct stat st;
    return path != NULL && stat(path, &st) == 0;
}

static void nx_cert_copy(char* dst, size_t cap, const char* src)
{
    if (dst == NULL || cap == 0u) {
        return;
    }
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    (void)snprintf(dst, cap, "%s", src);
}

static int nx_cert_join(char* out, size_t cap, const char* a, const char* b)
{
    int written;
    if (out == NULL || cap == 0u || a == NULL || b == NULL) {
        return 0;
    }
    written = snprintf(out, cap, "%s%c%s", a, NX_CERT_SEP, b);
    return written >= 0 && (size_t)written < cap;
}

static int nx_cert_mkdir_chain(const char* path)
{
    char buffer[NX_CERT_PATH_MAX];
    size_t i;
    size_t len;
    if (path == NULL || path[0] == '\0') {
        return 0;
    }
    nx_cert_copy(buffer, sizeof(buffer), path);
    len = strlen(buffer);
    for (i = 1u; i < len; ++i) {
        if (buffer[i] == '/' || buffer[i] == '\\') {
            char saved = buffer[i];
            buffer[i] = '\0';
            if (buffer[0] != '\0' && !(i == 2u && buffer[1] == ':')) {
                (void)NX_CERT_MKDIR(buffer);
            }
            buffer[i] = saved;
        }
    }
    (void)NX_CERT_MKDIR(buffer);
    return nx_cert_exists(buffer);
}

static void nx_cert_normalize_id(char* out, size_t cap, const char* value)
{
    size_t i;
    size_t j = 0u;
    int dash = 0;
    for (i = 0u; value != NULL && value[i] != '\0' && j + 1u < cap; ++i) {
        unsigned char ch = (unsigned char)value[i];
        if (isalnum(ch)) {
            out[j++] = (char)tolower(ch);
            dash = 0;
        } else if (!dash && j > 0u) {
            out[j++] = '-';
            dash = 1;
        }
    }
    while (j > 0u && out[j - 1u] == '-') {
        --j;
    }
    out[j] = '\0';
}

static int nx_cert_has_token(const char* line, const char* token)
{
    return line != NULL && token != NULL && strstr(line, token) != NULL;
}

static void nx_cert_parse_log(const char* log_path, NxPackageCertificationSummary* summary)
{
    FILE* file;
    char line[NX_CERT_LINE_MAX];
    int last_total = -1;
    int last_failed = -1;
    int last_not_run = -1;
    if (log_path == NULL || summary == NULL) {
        return;
    }
    file = fopen(log_path, "rb");
    if (file == NULL) {
        summary->errors_found++;
        return;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        int total;
        int failed;
        int not_run;
        if (nx_cert_has_token(line, "warning:") || nx_cert_has_token(line, "WARNING:")) {
            summary->warnings_found++;
        }
        if (nx_cert_has_token(line, " error:") || nx_cert_has_token(line, "FAILED:") ||
            nx_cert_has_token(line, "Errors while running CTest")) {
            summary->errors_found++;
        }
        if (sscanf(line, "%d%% tests passed, %d tests failed out of %d", &total, &failed, &not_run) == 3) {
            last_failed = failed;
            last_total = not_run;
        }
        if (strstr(line, "tests failed out of") != NULL &&
            sscanf(line, "%d tests failed out of %d", &failed, &total) == 2) {
            last_failed = failed;
            last_total = total;
        }
        if (strstr(line, "tests not run") != NULL &&
            sscanf(line, "%d tests not run", &not_run) == 1) {
            last_not_run = not_run;
        }
    }
    (void)fclose(file);
    if (last_total >= 0) {
        summary->tests_registered = last_total;
        summary->tests_executed = last_total - ((last_not_run > 0) ? last_not_run : 0);
        summary->tests_failed = (last_failed > 0) ? last_failed : 0;
        summary->tests_not_run = (last_not_run > 0) ? last_not_run : 0;
        summary->tests_passed = summary->tests_executed - summary->tests_failed;
    }
}

int NxPackageCertification_ValidateArtifacts(const char* repo_root,
                                             const char* package_dir,
                                             NxPackageCertificationSummary* summary)
{
    char manifest[NX_CERT_PATH_MAX];
    char line[NX_CERT_LINE_MAX];
    FILE* file;
    if (repo_root == NULL || package_dir == NULL || summary == NULL ||
        !nx_cert_join(manifest, sizeof(manifest), package_dir, "manifest.npkg")) {
        return 0;
    }
    file = fopen(manifest, "rb");
    if (file == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "artifact=", 9u) == 0) {
            char* value = line + 9;
            char* end = strpbrk(value, "\r\n");
            char path[NX_CERT_PATH_MAX];
            if (end != NULL) {
                *end = '\0';
            }
            summary->required_artifacts++;
            if (nx_cert_join(path, sizeof(path), repo_root, value) && nx_cert_exists(path)) {
                summary->artifacts_found++;
            } else {
                summary->artifacts_missing++;
            }
        }
    }
    (void)fclose(file);
    return summary->artifacts_missing == 0;
}

static void nx_cert_write_common(FILE* file,
                                 const NxPackageApplyResult* result,
                                 const NxPackageCertificationSummary* summary,
                                 time_t started_at,
                                 time_t finished_at,
                                 int machine_readable)
{
    long long elapsed = (long long)difftime(finished_at, started_at);
    const char* sep = machine_readable ? "=" : ": ";
    fprintf(file, "package_id%s%s\n", sep, result->package_id);
    fprintf(file, "package_version%s%s\n", sep, result->package_version);
    fprintf(file, "transaction_id%s%s\n", sep, result->transaction_id);
    fprintf(file, "verify_status%s%s\n", sep, result->verify_passed ? "PASS" : "FAIL");
    fprintf(file, "dependencies_status%s%s\n", sep, result->dependencies_passed ? "PASS" : "FAIL");
    fprintf(file, "install_status%s%s\n", sep, result->install_passed ? "PASS" : "FAIL");
    fprintf(file, "configure_status%s%s\n", sep, result->configure_passed ? "PASS" : "FAIL");
    fprintf(file, "build_status%s%s\n", sep, result->build_passed ? "PASS" : "FAIL");
    fprintf(file, "warning_gate_status%s%s\n", sep, result->warning_gate_passed ? "PASS" : "FAIL");
    fprintf(file, "tests_status%s%s\n", sep, result->tests_passed ? "PASS" : "FAIL");
    fprintf(file, "documentation_status%s%s\n", sep, result->documentation_passed ? "PASS" : "FAIL");
    fprintf(file, "rollback_status%s%s\n", sep, result->rolled_back ? "DONE" : "NOT_REQUIRED");
    fprintf(file, "failed_phase%s%s\n", sep, result->failed_phase[0] != '\0' ? result->failed_phase : "none");
    fprintf(file, "tests_registered%s%d\n", sep, summary->tests_registered);
    fprintf(file, "tests_executed%s%d\n", sep, summary->tests_executed);
    fprintf(file, "tests_passed%s%d\n", sep, summary->tests_passed);
    fprintf(file, "tests_failed%s%d\n", sep, summary->tests_failed);
    fprintf(file, "tests_not_run%s%d\n", sep, summary->tests_not_run);
    fprintf(file, "qa_repetitions%s%d\n", sep, summary->qa_repetitions);
    fprintf(file, "qa_repetitions_passed%s%d\n", sep, summary->qa_repetitions_passed);
    fprintf(file, "warnings_found%s%d\n", sep, summary->warnings_found);
    fprintf(file, "errors_found%s%d\n", sep, summary->errors_found);
    fprintf(file, "required_artifacts%s%d\n", sep, summary->required_artifacts);
    fprintf(file, "artifacts_found%s%d\n", sep, summary->artifacts_found);
    fprintf(file, "artifacts_missing%s%d\n", sep, summary->artifacts_missing);
    fprintf(file, "coverage_status%sNOT_MEASURED\n", sep);
    fprintf(file, "coverage_delta%sUNKNOWN\n", sep);
    fprintf(file, "start_epoch%s%lld\n", sep, (long long)started_at);
    fprintf(file, "end_epoch%s%lld\n", sep, (long long)finished_at);
    fprintf(file, "elapsed_seconds%s%lld\n", sep, elapsed);
    fprintf(file, "final_status%s%s\n", sep, summary->final_status);
    fprintf(file, "release_recommendation%s%s\n", sep, summary->release_recommendation);
    fprintf(file, "apply_log%s%s\n", sep, result->apply_log_path);
    fprintf(file, "message%s%s\n", sep, result->message);
}

int NxPackageCertification_Generate(const char* repo_root,
                                    const char* package_dir,
                                    const NxPackageApplyResult* apply_result,
                                    time_t started_at,
                                    time_t finished_at,
                                    int qa_repetitions,
                                    int qa_repetitions_passed,
                                    NxPackageCertificationSummary* out_summary)
{
    NxPackageCertificationSummary summary;
    char safe_id[128];
    char root_dir[NX_CERT_PATH_MAX];
    char package_dir_out[NX_CERT_PATH_MAX];
    FILE* nxcert;
    FILE* text;
    int artifacts_ok;
    if (repo_root == NULL || package_dir == NULL || apply_result == NULL || out_summary == NULL) {
        return 0;
    }
    memset(&summary, 0, sizeof(summary));
    summary.qa_repetitions = qa_repetitions;
    summary.qa_repetitions_passed = qa_repetitions_passed;
    nx_cert_parse_log(apply_result->apply_log_path, &summary);
    artifacts_ok = NxPackageCertification_ValidateArtifacts(repo_root, package_dir, &summary);
    if (apply_result->success && artifacts_ok && qa_repetitions_passed == qa_repetitions &&
        summary.warnings_found == 0 && summary.tests_failed == 0 && summary.tests_not_run == 0) {
        nx_cert_copy(summary.final_status, sizeof(summary.final_status), "CERTIFIED");
        nx_cert_copy(summary.release_recommendation, sizeof(summary.release_recommendation), "APPROVE_GIT");
    } else if (apply_result->rolled_back) {
        nx_cert_copy(summary.final_status, sizeof(summary.final_status), "ROLLED_BACK");
        nx_cert_copy(summary.release_recommendation, sizeof(summary.release_recommendation), "DO_NOT_COMMIT");
    } else {
        nx_cert_copy(summary.final_status, sizeof(summary.final_status), "REJECTED");
        nx_cert_copy(summary.release_recommendation, sizeof(summary.release_recommendation), "DO_NOT_COMMIT");
    }
    nx_cert_normalize_id(safe_id, sizeof(safe_id), apply_result->package_id);
    if (!nx_cert_join(root_dir, sizeof(root_dir), repo_root, "Knowledge/NCOS/Certification") ||
        !nx_cert_join(package_dir_out, sizeof(package_dir_out), root_dir, safe_id) ||
        !nx_cert_mkdir_chain(package_dir_out) ||
        !nx_cert_join(summary.nxcert_path, sizeof(summary.nxcert_path), package_dir_out, "certification-report.nxcert") ||
        !nx_cert_join(summary.text_path, sizeof(summary.text_path), package_dir_out, "certification-report.txt")) {
        return 0;
    }
    nxcert = fopen(summary.nxcert_path, "wb");
    text = fopen(summary.text_path, "wb");
    if (nxcert == NULL || text == NULL) {
        if (nxcert != NULL) {
            (void)fclose(nxcert);
        }
        if (text != NULL) {
            (void)fclose(text);
        }
        return 0;
    }
    fputs("nxcert/1\n", nxcert);
    nx_cert_write_common(nxcert, apply_result, &summary, started_at, finished_at, 1);
    fputs("NEXIORA INSTALLATION CERTIFICATION\n\n", text);
    nx_cert_write_common(text, apply_result, &summary, started_at, finished_at, 0);
    (void)fclose(nxcert);
    (void)fclose(text);
    *out_summary = summary;
    return 1;
}
