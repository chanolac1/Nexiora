#include "Nexiora/NCOS/NxPatchApprovalEngine.h"

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
    snprintf(dst, dst_size, "%s", src);
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

static int nx_ensure_dirs(const char* root, char* approvals_dir, size_t approvals_dir_size)
{
    char knowledge[512];
    char ncos[512];
    if (!root || !approvals_dir) return 0;
    (void)NX_MKDIR(root);
    if (!nx_join(knowledge, sizeof(knowledge), root, "Knowledge")) return 0;
    (void)NX_MKDIR(knowledge);
    if (!nx_join(ncos, sizeof(ncos), knowledge, "NCOS")) return 0;
    (void)NX_MKDIR(ncos);
    if (!nx_join(approvals_dir, approvals_dir_size, ncos, "PatchApprovals")) return 0;
    (void)NX_MKDIR(approvals_dir);
    return 1;
}

static int nx_approval_path(const char* root, const char* run_id, char* path, size_t path_size, char* safe_id, size_t safe_id_size)
{
    char dir[512];
    char filename[192];
    nx_normalize_id(safe_id, safe_id_size, run_id);
    if (!nx_ensure_dirs(root, dir, sizeof(dir))) return 0;
    snprintf(filename, sizeof(filename), "%s.approval.md", safe_id);
    filename[sizeof(filename) - 1] = '\0';
    return nx_join(path, path_size, dir, filename);
}

static int nx_write_record(const char* path,
                           const char* safe_id,
                           const char* proposal_path,
                           const char* status,
                           const char* reviewer,
                           const char* reason)
{
    FILE* f;
    time_t now = time(NULL);
    f = fopen(path, "wb");
    if (!f) return 0;
    fprintf(f, "# NCOS Patch Approval Record\n\n");
    fprintf(f, "Run ID: %s\n", safe_id);
    fprintf(f, "Status: %s\n", status ? status : "pending");
    fprintf(f, "Reviewer: %s\n", reviewer && reviewer[0] ? reviewer : "human_required");
    fprintf(f, "Timestamp: %lld\n", (long long)now);
    fprintf(f, "Proposal: %s\n\n", proposal_path && proposal_path[0] ? proposal_path : "unknown");
    fprintf(f, "## Policy\n\n");
    fprintf(f, "Nexiora may propose patches, but promotion requires explicit human approval.\n\n");
    if (reason && reason[0]) {
        fprintf(f, "## Review Reason\n\n%s\n\n", reason);
    }
    fprintf(f, "## Decision Gate\n\n");
    if (status && strcmp(status, "approved") == 0) fprintf(f, "Decision: approved_for_application\n");
    else if (status && strcmp(status, "rejected") == 0) fprintf(f, "Decision: rejected_by_human\n");
    else fprintf(f, "Decision: waiting_for_human_approval\n");
    fclose(f);
    return 1;
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
            line[strcspn(line, "\r\n")] = '\0';
            nx_copy(out, out_size, v);
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

static void nx_fill_result(NxPatchApprovalResult* out,
                           const char* safe_id,
                           const char* status,
                           const char* path,
                           const char* proposal,
                           const char* reviewer)
{
    if (!out) return;
    out->ok = 1;
    nx_copy(out->run_id, sizeof(out->run_id), safe_id);
    nx_copy(out->status, sizeof(out->status), status);
    nx_copy(out->approval_path, sizeof(out->approval_path), path);
    nx_copy(out->proposal_path, sizeof(out->proposal_path), proposal);
    nx_copy(out->reviewer, sizeof(out->reviewer), reviewer);
    snprintf(out->summary, sizeof(out->summary), "Patch approval status for %s is %s.", safe_id, status);
    out->summary[sizeof(out->summary) - 1] = '\0';
}

int NxPatchApproval_Request(const char* root_path,
                            const char* run_id,
                            const char* proposal_path,
                            NxPatchApprovalResult* out_result)
{
    char safe_id[128];
    char path[512];
    if (!out_result) return 0;
    nx_zero(out_result, sizeof(*out_result));
    if (!root_path || !proposal_path) return 0;
    if (!nx_approval_path(root_path, run_id, path, sizeof(path), safe_id, sizeof(safe_id))) return 0;
    if (!nx_write_record(path, safe_id, proposal_path, "pending", "human_required", "Awaiting explicit approval.")) return 0;
    nx_fill_result(out_result, safe_id, "pending", path, proposal_path, "human_required");
    return 1;
}

int NxPatchApproval_Approve(const char* root_path,
                            const char* run_id,
                            const char* reviewer,
                            NxPatchApprovalResult* out_result)
{
    char safe_id[128];
    char path[512];
    char proposal[512];
    if (!out_result) return 0;
    nx_zero(out_result, sizeof(*out_result));
    if (!root_path) return 0;
    if (!nx_approval_path(root_path, run_id, path, sizeof(path), safe_id, sizeof(safe_id))) return 0;
    if (!nx_read_value(path, "Proposal", proposal, sizeof(proposal))) nx_copy(proposal, sizeof(proposal), "unknown");
    if (!nx_write_record(path, safe_id, proposal, "approved", reviewer && reviewer[0] ? reviewer : "human", "Approved by human reviewer.")) return 0;
    nx_fill_result(out_result, safe_id, "approved", path, proposal, reviewer && reviewer[0] ? reviewer : "human");
    return 1;
}

int NxPatchApproval_Reject(const char* root_path,
                           const char* run_id,
                           const char* reviewer,
                           const char* reason,
                           NxPatchApprovalResult* out_result)
{
    char safe_id[128];
    char path[512];
    char proposal[512];
    if (!out_result) return 0;
    nx_zero(out_result, sizeof(*out_result));
    if (!root_path) return 0;
    if (!nx_approval_path(root_path, run_id, path, sizeof(path), safe_id, sizeof(safe_id))) return 0;
    if (!nx_read_value(path, "Proposal", proposal, sizeof(proposal))) nx_copy(proposal, sizeof(proposal), "unknown");
    if (!nx_write_record(path, safe_id, proposal, "rejected", reviewer && reviewer[0] ? reviewer : "human", reason && reason[0] ? reason : "Rejected by human reviewer.")) return 0;
    nx_fill_result(out_result, safe_id, "rejected", path, proposal, reviewer && reviewer[0] ? reviewer : "human");
    return 1;
}

int NxPatchApproval_Status(const char* root_path,
                           const char* run_id,
                           NxPatchApprovalResult* out_result)
{
    char safe_id[128];
    char path[512];
    char status[64];
    char proposal[512];
    char reviewer[128];
    if (!out_result) return 0;
    nx_zero(out_result, sizeof(*out_result));
    if (!root_path) return 0;
    if (!nx_approval_path(root_path, run_id, path, sizeof(path), safe_id, sizeof(safe_id))) return 0;
    if (!nx_read_value(path, "Status", status, sizeof(status))) return 0;
    if (!nx_read_value(path, "Proposal", proposal, sizeof(proposal))) nx_copy(proposal, sizeof(proposal), "unknown");
    if (!nx_read_value(path, "Reviewer", reviewer, sizeof(reviewer))) nx_copy(reviewer, sizeof(reviewer), "unknown");
    nx_fill_result(out_result, safe_id, status, path, proposal, reviewer);
    return 1;
}
