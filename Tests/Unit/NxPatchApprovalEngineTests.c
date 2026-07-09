#include "Nexiora/NCOS/NxPatchApprovalEngine.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static int failures = 0;

static void check(int condition, const char* message)
{
    if (!condition) {
        printf("FAIL: %s\n", message);
        failures++;
    }
}

static int file_contains(const char* path, const char* needle)
{
    char line[512];
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, needle)) { fclose(f); return 1; }
    }
    fclose(f);
    return 0;
}

int main(void)
{
    const char* root = "Build/NxPatchApprovalEngineTestsSandbox";
    const char* proposal = "Build/NxPatchApprovalEngineTestsSandbox/sample.patch.md";
    NxPatchApprovalResult r;
    FILE* f;

    (void)NX_MKDIR("Build");
    (void)NX_MKDIR(root);
    f = fopen(proposal, "wb");
    check(f != NULL, "should create sample proposal");
    if (f) {
        fprintf(f, "# NCOS Safe Patch Proposal\nStatus: waiting_for_human_approval\n");
        fclose(f);
    }

    check(NxPatchApproval_Request(root, "Patch Approval 001", proposal, &r) == 1, "request should succeed");
    check(r.ok == 1, "request result should be ok");
    check(strcmp(r.run_id, "patch_approval_001") == 0, "run id should be normalized");
    check(strcmp(r.status, "pending") == 0, "status should be pending");
    check(file_contains(r.approval_path, "waiting_for_human_approval"), "approval should require human gate");

    check(NxPatchApproval_Status(root, "Patch Approval 001", &r) == 1, "status should succeed");
    check(strcmp(r.status, "pending") == 0, "status should read pending");

    check(NxPatchApproval_Approve(root, "Patch Approval 001", "Jorge", &r) == 1, "approve should succeed");
    check(strcmp(r.status, "approved") == 0, "status should be approved");
    check(file_contains(r.approval_path, "approved_for_application"), "approval should contain approved gate");

    check(NxPatchApproval_Status(root, "Patch Approval 001", &r) == 1, "approved status should succeed");
    check(strcmp(r.status, "approved") == 0, "status should read approved");

    check(NxPatchApproval_Reject(root, "Patch Approval 002", "Jorge", "Needs more tests", &r) == 1, "reject should succeed even without prior request");
    check(strcmp(r.status, "rejected") == 0, "status should be rejected");
    check(file_contains(r.approval_path, "rejected_by_human"), "rejection should contain rejected gate");

    return failures == 0 ? 0 : 1;
}
