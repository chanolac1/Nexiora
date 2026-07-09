#include "Nexiora/NCOS/NxSafePatchEngine.h"

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
    if (!dst || dst_size == 0 || !a || !b) return 0;
    if (a[0] != '\0') {
        size_t len = strlen(a);
        if (len > 0 && a[len - 1] != '/' && a[len - 1] != '\\') sep = "/";
    }
    int written = snprintf(dst, dst_size, "%s%s%s", a, sep, b);
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

static int nx_file_contains(const char* path, const char* needle)
{
    char line[1024];
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, needle)) { fclose(f); return 1; }
    }
    fclose(f);
    return 0;
}

static int nx_ensure_dirs(const char* root, char* out_dir, size_t out_dir_size)
{
    char knowledge[512];
    char ncos[512];
    if (!nx_join(knowledge, sizeof(knowledge), root, "Knowledge")) return 0;
    (void)NX_MKDIR(knowledge);
    if (!nx_join(ncos, sizeof(ncos), knowledge, "NCOS")) return 0;
    (void)NX_MKDIR(ncos);
    if (!nx_join(out_dir, out_dir_size, ncos, "PatchProposals")) return 0;
    (void)NX_MKDIR(out_dir);
    return 1;
}

static int nx_write_patch_proposal(FILE* out, const char* run_id, const char* source_path)
{
    int changes = 0;
    int has_semicolon_error = nx_file_contains(source_path, "expected ';'") || nx_file_contains(source_path, "expected ';' before");
    int has_implicit = nx_file_contains(source_path, "implicit declaration of function");
    int has_missing_file = nx_file_contains(source_path, "Cannot find source file") || nx_file_contains(source_path, "No such file or directory");
    int has_failed_test = nx_file_contains(source_path, "tests FAILED") || nx_file_contains(source_path, "***Failed");

    time_t now = time(NULL);
    fprintf(out, "# NCOS Safe Patch Proposal\n\n");
    fprintf(out, "Run ID: %s\n", run_id);
    fprintf(out, "Generated: %lld\n", (long long)now);
    fprintf(out, "Source analysis/proposal: %s\n\n", source_path);
    fprintf(out, "## Safety Policy\n\n");
    fprintf(out, "This proposal is review-only. Nexiora does not modify production source automatically. Human approval is required before applying any change.\n\n");

    fprintf(out, "## Proposed Actions\n\n");
    if (has_semicolon_error) {
        fprintf(out, "- [manual-review] Inspect the reported C source line and add the missing semicolon or syntax terminator.\n");
        fprintf(out, "  - Category: C syntax\n  - Risk: low\n  - Verification: rebuild and rerun focused test.\n");
        changes++;
    }
    if (has_implicit) {
        fprintf(out, "- [manual-review] Add the required standard header for the implicitly declared function, or replace the call with a portable wrapper.\n");
        fprintf(out, "  - Category: C2x portability\n  - Risk: low/medium\n  - Verification: compile with -Wall -Wextra -Wpedantic.\n");
        changes++;
    }
    if (has_missing_file) {
        fprintf(out, "- [manual-review] Restore missing source/test files or repair CMake integration to reference existing files only.\n");
        fprintf(out, "  - Category: build integration\n  - Risk: medium\n  - Verification: cmake configure + full suite.\n");
        changes++;
    }
    if (has_failed_test) {
        fprintf(out, "- [manual-review] Run the failing test with --output-on-failure, isolate persistent state, then add regression coverage.\n");
        fprintf(out, "  - Category: test failure\n  - Risk: medium\n  - Verification: focused test + complete ctest.\n");
        changes++;
    }
    if (changes == 0) {
        fprintf(out, "- [no-op] No known actionable build pattern was detected. Preserve the analysis for human review.\n");
    }

    fprintf(out, "\n## Required Verification\n\n");
    fprintf(out, "1. cmake --preset windows-msvc-release\n");
    fprintf(out, "2. cmake --build --preset release\n");
    fprintf(out, "3. ctest --test-dir .\\Build\\windows-msvc-release --output-on-failure\n\n");
    fprintf(out, "## Decision Gate\n\n");
    fprintf(out, "Status: waiting_for_human_approval\n");
    return changes;
}

int NxSafePatch_CreateProposal(const char* root_path,
                               const char* run_id,
                               const char* analysis_or_proposal_path,
                               NxSafePatchResult* out_result)
{
    char safe_id[128];
    char out_dir[512];
    char filename[192];
    FILE* out;
    int changes;

    if (!out_result) return 0;
    nx_zero(out_result, sizeof(*out_result));
    if (!root_path || !analysis_or_proposal_path) return 0;

    nx_normalize_id(safe_id, sizeof(safe_id), run_id && run_id[0] ? run_id : "patch");
    nx_copy(out_result->run_id, sizeof(out_result->run_id), safe_id);

    if (!nx_ensure_dirs(root_path, out_dir, sizeof(out_dir))) return 0;
    snprintf(filename, sizeof(filename), "%s.patch.md", safe_id);
    filename[sizeof(filename) - 1] = '\0';
    if (!nx_join(out_result->proposal_path, sizeof(out_result->proposal_path), out_dir, filename)) return 0;

    out = fopen(out_result->proposal_path, "wb");
    if (!out) return 0;
    changes = nx_write_patch_proposal(out, safe_id, analysis_or_proposal_path);
    fclose(out);

    out_result->ok = 1;
    out_result->proposed_changes = changes;
    out_result->requires_human_approval = 1;
    snprintf(out_result->summary, sizeof(out_result->summary),
             "Patch proposal generated with %d proposed action(s); human approval required.", changes);
    out_result->summary[sizeof(out_result->summary) - 1] = '\0';
    return 1;
}
