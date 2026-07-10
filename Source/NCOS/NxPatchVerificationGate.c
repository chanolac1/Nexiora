#include "Nexiora/NCOS/NxPatchVerificationGate.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static void nx_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0U) {
        return;
    }

    if (src == NULL) {
        src = "";
    }

    size_t i = 0U;
    while (src[i] != '\0' && i + 1U < dst_size) {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
}

static void nx_norm(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0U) {
        return;
    }

    if (src == NULL || src[0] == '\0') {
        nx_copy(dst, dst_size, "verification");
        return;
    }

    size_t j = 0U;
    for (size_t i = 0U; src[i] != '\0' && j + 1U < dst_size; ++i) {
        const unsigned char c = (unsigned char)src[i];
        if (isalnum(c) != 0) {
            dst[j++] = (char)tolower(c);
        } else if (c == '-' || c == '_') {
            dst[j++] = '_';
        }
    }

    if (j == 0U) {
        nx_copy(dst, dst_size, "verification");
        return;
    }

    dst[j] = '\0';
}

static int nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    if (dst == NULL || dst_size == 0U || a == NULL || b == NULL) {
        return 0;
    }

    const size_t a_len = strlen(a);
    const size_t b_len = strlen(b);
    const int needs_sep = (a_len > 0U && a[a_len - 1U] != '/' && a[a_len - 1U] != '\\') ? 1 : 0;
    const size_t required = a_len + (needs_sep != 0 ? 1U : 0U) + b_len + 1U;

    if (required > dst_size) {
        dst[0] = '\0';
        return 0;
    }

    size_t pos = 0U;
    if (a_len > 0U) {
        memcpy(dst + pos, a, a_len);
        pos += a_len;
    }

    if (needs_sep != 0) {
        dst[pos++] = '/';
    }

    if (b_len > 0U) {
        memcpy(dst + pos, b, b_len);
        pos += b_len;
    }

    dst[pos] = '\0';
    return 1;
}

static int nx_exists(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}

static void nx_mkdir_chain(const char* path)
{
    if (path == NULL || path[0] == '\0') {
        return;
    }

    char tmp[512];
    nx_copy(tmp, sizeof(tmp), path);

    for (char* cursor = tmp; *cursor != '\0'; ++cursor) {
        if (*cursor == '/' || *cursor == '\\') {
            const char old = *cursor;
            *cursor = '\0';
            if (tmp[0] != '\0') {
                (void)NX_MKDIR(tmp);
            }
            *cursor = old;
        }
    }

    (void)NX_MKDIR(tmp);
}

static int nx_read_contains(const char* path, const char* needle)
{
    if (path == NULL || needle == NULL) {
        return 0;
    }

    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }

    char line[1024];
    int found = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, needle) != NULL) {
            found = 1;
            break;
        }
    }

    fclose(file);
    return found;
}

static int nx_dirs(const char* root, char* ncos, size_t ncos_size, char* verification, size_t verification_size)
{
    char knowledge[512];
    const char* base = (root != NULL && root[0] != '\0') ? root : ".";

    if (!nx_join(knowledge, sizeof(knowledge), base, "Knowledge")) {
        return 0;
    }

    nx_mkdir_chain(knowledge);

    if (!nx_join(ncos, ncos_size, knowledge, "NCOS")) {
        return 0;
    }

    nx_mkdir_chain(ncos);

    if (!nx_join(verification, verification_size, ncos, "PatchVerification")) {
        return 0;
    }

    nx_mkdir_chain(verification);
    return 1;
}

static int nx_application_path(const char* root, const char* id, char* out, size_t out_size)
{
    char safe[128];
    char knowledge[512];
    char ncos[512];
    char applications[512];
    char dir[512];
    const char* base = (root != NULL && root[0] != '\0') ? root : ".";

    nx_norm(safe, sizeof(safe), id);

    return nx_join(knowledge, sizeof(knowledge), base, "Knowledge") &&
           nx_join(ncos, sizeof(ncos), knowledge, "NCOS") &&
           nx_join(applications, sizeof(applications), ncos, "PatchApplications") &&
           nx_join(dir, sizeof(dir), applications, safe) &&
           nx_join(out, out_size, dir, "application.md");
}

static int nx_approval_path(const char* root, const char* id, char* out, size_t out_size)
{
    char safe[128];
    char knowledge[512];
    char ncos[512];
    char approvals[512];
    char dir[512];
    const char* base = (root != NULL && root[0] != '\0') ? root : ".";

    nx_norm(safe, sizeof(safe), id);

    return nx_join(knowledge, sizeof(knowledge), base, "Knowledge") &&
           nx_join(ncos, sizeof(ncos), knowledge, "NCOS") &&
           nx_join(approvals, sizeof(approvals), ncos, "PatchApprovals") &&
           nx_join(dir, sizeof(dir), approvals, safe) &&
           nx_join(out, out_size, dir, "approval.md");
}

static int nx_report_path(const char* root, const char* id, char* out, size_t out_size)
{
    char safe[128];
    char ncos[512];
    char verification[512];
    char dir[512];

    nx_norm(safe, sizeof(safe), id);

    if (!nx_dirs(root, ncos, sizeof(ncos), verification, sizeof(verification))) {
        return 0;
    }

    if (!nx_join(dir, sizeof(dir), verification, safe)) {
        return 0;
    }

    nx_mkdir_chain(dir);
    return nx_join(out, out_size, dir, "verification.md");
}

static void nx_init(NxPatchVerificationResult* result, const char* id)
{
    if (result == NULL) {
        return;
    }

    memset(result, 0, sizeof(*result));
    nx_norm(result->id, sizeof(result->id), id);
    nx_copy(result->status, sizeof(result->status), "unknown");
}

int NxPatchVerificationGate_Verify(const char* root, const char* id, NxPatchVerificationResult* out)
{
    NxPatchVerificationResult result;
    nx_init(&result, id);

    char approval_path[512];
    char application_path[512];
    char report_path[512];

    if (!nx_approval_path(root, id, approval_path, sizeof(approval_path)) ||
        !nx_application_path(root, id, application_path, sizeof(application_path)) ||
        !nx_report_path(root, id, report_path, sizeof(report_path))) {
        return 0;
    }

    nx_copy(result.application_record, sizeof(result.application_record), application_path);
    nx_copy(result.verification_report, sizeof(result.verification_report), report_path);

    result.approval_found = nx_exists(approval_path) && nx_read_contains(approval_path, "APPROVED");
    result.application_found = nx_exists(application_path);
    result.tests_passed = nx_read_contains(application_path, "Tests: PASSED") ||
                          nx_read_contains(application_path, "tests: passed") ||
                          nx_read_contains(application_path, "Validation: PASSED");
    result.warnings = nx_read_contains(application_path, "warning") || nx_read_contains(application_path, "Warning");
    result.errors = nx_read_contains(application_path, "error") ||
                    nx_read_contains(application_path, "FAILED") ||
                    nx_read_contains(application_path, "Failed");
    result.promotable = result.approval_found && result.application_found && result.tests_passed && !result.errors;

    nx_copy(result.status, sizeof(result.status), result.promotable ? "PROMOTABLE" : "BLOCKED");

    FILE* file = fopen(report_path, "wb");
    if (file == NULL) {
        return 0;
    }

    fprintf(file, "# NCOS Patch Verification\n\n");
    fprintf(file, "Run ID: %s\n\n", result.id);
    fprintf(file, "Status: %s\n\n", result.status);
    fprintf(file, "Approval found: %s\n", result.approval_found ? "yes" : "no");
    fprintf(file, "Application found: %s\n", result.application_found ? "yes" : "no");
    fprintf(file, "Tests passed: %s\n", result.tests_passed ? "yes" : "no");
    fprintf(file, "Errors detected: %d\n", result.errors);
    fprintf(file, "Warnings detected: %d\n\n", result.warnings);
    fprintf(file, "Application record: %s\n\n", result.application_record);
    fprintf(file, "Decision: %s\n", result.promotable ?
            "Candidate can be promoted after human review." :
            "Candidate is blocked until missing evidence is resolved.");
    fclose(file);

    if (out != NULL) {
        *out = result;
    }

    return 1;
}

int NxPatchVerificationGate_Status(const char* root, const char* id, NxPatchVerificationResult* out)
{
    NxPatchVerificationResult result;
    nx_init(&result, id);

    if (!nx_report_path(root, id, result.verification_report, sizeof(result.verification_report))) {
        return 0;
    }

    if (!nx_exists(result.verification_report)) {
        return 0;
    }

    result.promotable = nx_read_contains(result.verification_report, "Status: PROMOTABLE");
    nx_copy(result.status, sizeof(result.status), result.promotable ? "PROMOTABLE" : "BLOCKED");

    if (out != NULL) {
        *out = result;
    }

    return 1;
}
