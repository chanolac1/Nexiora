#include "Nexiora/NCOS/NxApprovedPatchApplyEngine.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static void nx_zero(void* p, size_t n)
{
    unsigned char* b = (unsigned char*)p;
    while (n--) *b++ = 0;
}

static void nx_copy(char* dst, size_t dst_size, const char* src)
{
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    (void)snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static int nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "";
    int written;
    if (!dst || dst_size == 0 || !a || !b) return 0;
    if (a[0] != '\0') {
        size_t len = strlen(a);
        if (len > 0 && a[len - 1] != '/' && a[len - 1] != '\\') sep = "/";
    }
    written = snprintf(dst, dst_size, "%s%s%s", a, sep, b);
    dst[dst_size - 1] = '\0';
    return written > 0 && (size_t)written < dst_size;
}

static void nx_normalize_id(char* out, size_t out_size, const char* in)
{
    size_t j = 0;
    if (!out || out_size == 0) return;
    if (!in || in[0] == '\0') in = "patch";
    for (size_t i = 0; in[i] && j + 1 < out_size; ++i) {
        unsigned char c = (unsigned char)in[i];
        if (isalnum(c)) out[j++] = (char)tolower(c);
        else if ((c == '-' || c == '_' || c == ' ') && j > 0 && out[j - 1] != '_') out[j++] = '_';
    }
    if (j == 0) out[j++] = 'p';
    while (j > 1 && out[j - 1] == '_') --j;
    out[j] = '\0';
}

static int nx_ensure_base_dirs(const char* root, char* ncos_dir, size_t ncos_dir_size)
{
    char knowledge[512];
    if (!root || !ncos_dir) return 0;
    (void)NX_MKDIR(root);
    if (!nx_join(knowledge, sizeof(knowledge), root, "Knowledge")) return 0;
    (void)NX_MKDIR(knowledge);
    if (!nx_join(ncos_dir, ncos_dir_size, knowledge, "NCOS")) return 0;
    (void)NX_MKDIR(ncos_dir);
    return 1;
}

static int nx_approval_path(const char* root, const char* run_id, char* path, size_t path_size, char* safe_id, size_t safe_id_size)
{
    char ncos[512];
    char approvals[512];
    char filename[192];
    nx_normalize_id(safe_id, safe_id_size, run_id);
    if (!nx_ensure_base_dirs(root, ncos, sizeof(ncos))) return 0;
    if (!nx_join(approvals, sizeof(approvals), ncos, "PatchApprovals")) return 0;
    (void)NX_MKDIR(approvals);
    (void)snprintf(filename, sizeof(filename), "%s.approval.md", safe_id);
    filename[sizeof(filename) - 1] = '\0';
    return nx_join(path, path_size, approvals, filename);
}

static int nx_application_path(const char* root, const char* safe_id, char* path, size_t path_size)
{
    char ncos[512];
    char applications[512];
    char filename[192];
    if (!nx_ensure_base_dirs(root, ncos, sizeof(ncos))) return 0;
    if (!nx_join(applications, sizeof(applications), ncos, "AppliedPatches")) return 0;
    (void)NX_MKDIR(applications);
    (void)snprintf(filename, sizeof(filename), "%s.applied.md", safe_id ? safe_id : "patch");
    filename[sizeof(filename) - 1] = '\0';
    return nx_join(path, path_size, applications, filename);
}

static int nx_read_value(const char* path, const char* key, char* out, size_t out_size)
{
    char line[1024];
    size_t key_len;
    FILE* f;
    if (!path || !key || !out || out_size == 0) return 0;
    out[0] = '\0';
    key_len = strlen(key);
    f = fopen(path, "rb");
    if (!f) return 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, key, key_len) == 0) {
            char* v = line + key_len;
            while (*v == ' ' || *v == '\t' || *v == ':') ++v;
            v[strcspn(v, "\r\n")] = '\0';
            nx_copy(out, out_size, v);
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

static int nx_file_exists(const char* path)
{
    FILE* f;
    if (!path || !path[0]) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static int nx_write_application_record(const char* application_path,
                                       const char* safe_id,
                                       const char* approval_path,
                                       const char* proposal_path,
                                       const char* status,
                                       const char* summary)
{
    FILE* f;
    time_t now = time(NULL);
    f = fopen(application_path, "wb");
    if (!f) return 0;
    fprintf(f, "# NCOS Approved Patch Application Record\n\n");
    fprintf(f, "Run ID: %s\n", safe_id ? safe_id : "patch");
    fprintf(f, "Status: %s\n", status ? status : "unknown");
    fprintf(f, "Timestamp: %lld\n", (long long)now);
    fprintf(f, "Approval: %s\n", approval_path ? approval_path : "unknown");
    fprintf(f, "Proposal: %s\n\n", proposal_path ? proposal_path : "unknown");
    fprintf(f, "## Safety Policy\n\n");
    fprintf(f, "This engine does not promote changes directly to the runtime. It records an approved, reviewable application artifact.\n\n");
    fprintf(f, "## Result\n\n%s\n", summary ? summary : "Patch application record generated.");
    fclose(f);
    return 1;
}

static void nx_fill(NxApprovedPatchApplyResult* out,
                    const char* safe_id,
                    int ok,
                    int approved,
                    const char* status,
                    const char* approval_path,
                    const char* proposal_path,
                    const char* application_path,
                    const char* summary)
{
    if (!out) return;
    out->ok = ok;
    out->approved = approved;
    nx_copy(out->run_id, sizeof(out->run_id), safe_id);
    nx_copy(out->status, sizeof(out->status), status);
    nx_copy(out->approval_path, sizeof(out->approval_path), approval_path);
    nx_copy(out->proposal_path, sizeof(out->proposal_path), proposal_path);
    nx_copy(out->application_path, sizeof(out->application_path), application_path);
    nx_copy(out->summary, sizeof(out->summary), summary);
}

int NxApprovedPatchApply_Run(const char* root_path,
                             const char* run_id,
                             NxApprovedPatchApplyResult* out_result)
{
    char safe_id[128];
    char approval_path[512];
    char application_path[512];
    char status[64];
    char proposal[512];
    char summary[512];
    if (!out_result) return 0;
    nx_zero(out_result, sizeof(*out_result));
    if (!root_path) return 0;
    if (!nx_approval_path(root_path, run_id, approval_path, sizeof(approval_path), safe_id, sizeof(safe_id))) return 0;
    if (!nx_application_path(root_path, safe_id, application_path, sizeof(application_path))) return 0;
    if (!nx_read_value(approval_path, "Status", status, sizeof(status))) {
        nx_fill(out_result, safe_id, 0, 0, "missing_approval", approval_path, "", application_path, "No approval record was found.");
        return 0;
    }
    if (!nx_read_value(approval_path, "Proposal", proposal, sizeof(proposal))) nx_copy(proposal, sizeof(proposal), "unknown");
    if (strcmp(status, "approved") != 0) {
        (void)snprintf(summary, sizeof(summary), "Patch was not applied because approval status is '%s'.", status);
        summary[sizeof(summary) - 1] = '\0';
        nx_fill(out_result, safe_id, 0, 0, status, approval_path, proposal, application_path, summary);
        return 0;
    }
    if (!nx_file_exists(proposal)) {
        nx_fill(out_result, safe_id, 0, 1, "approved_but_missing_proposal", approval_path, proposal, application_path, "Approval exists, but the proposal artifact was not found.");
        return 0;
    }
    (void)snprintf(summary, sizeof(summary), "Approved patch '%s' was staged as a reviewable application artifact.", safe_id);
    summary[sizeof(summary) - 1] = '\0';
    if (!nx_write_application_record(application_path, safe_id, approval_path, proposal, "staged_for_review", summary)) return 0;
    nx_fill(out_result, safe_id, 1, 1, "staged_for_review", approval_path, proposal, application_path, summary);
    return 1;
}

int NxApprovedPatchApply_Status(const char* root_path,
                                const char* run_id,
                                NxApprovedPatchApplyResult* out_result)
{
    char safe_id[128];
    char approval_path[512];
    char application_path[512];
    char proposal[512];
    char status[64];
    if (!out_result) return 0;
    nx_zero(out_result, sizeof(*out_result));
    if (!root_path) return 0;
    if (!nx_approval_path(root_path, run_id, approval_path, sizeof(approval_path), safe_id, sizeof(safe_id))) return 0;
    if (!nx_application_path(root_path, safe_id, application_path, sizeof(application_path))) return 0;
    if (!nx_read_value(approval_path, "Status", status, sizeof(status))) {
        nx_fill(out_result, safe_id, 0, 0, "missing_approval", approval_path, "", application_path, "No approval record was found.");
        return 0;
    }
    if (!nx_read_value(approval_path, "Proposal", proposal, sizeof(proposal))) nx_copy(proposal, sizeof(proposal), "unknown");
    if (nx_file_exists(application_path)) {
        nx_fill(out_result, safe_id, 1, strcmp(status, "approved") == 0, "staged_for_review", approval_path, proposal, application_path, "Approved patch has a staged application record.");
        return 1;
    }
    nx_fill(out_result, safe_id, 0, strcmp(status, "approved") == 0, status, approval_path, proposal, application_path, "Patch has not been staged for application yet.");
    return 0;
}
