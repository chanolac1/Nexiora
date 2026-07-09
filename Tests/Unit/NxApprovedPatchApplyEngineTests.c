#include "Nexiora/NCOS/NxApprovedPatchApplyEngine.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int failures = 0;

static void expect_true(int cond, const char* msg)
{
    if (!cond) {
        printf("FAIL: %s\n", msg);
        failures++;
    }
}

static void write_file(const char* path, const char* text)
{
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fputs(text, f);
    fclose(f);
}

int main(void)
{
    const char* root = "Build/NxApprovedPatchApplyEngineTestsSandbox";
    const char* proposal = "Build/NxApprovedPatchApplyEngineTestsSandbox/proposal.patch.md";
    NxApprovedPatchApplyResult r;

    system("if exist Build\\NxApprovedPatchApplyEngineTestsSandbox rmdir /s /q Build\\NxApprovedPatchApplyEngineTestsSandbox >nul 2>nul");
    system("mkdir Build\\NxApprovedPatchApplyEngineTestsSandbox >nul 2>nul");

    write_file(proposal, "# Patch Proposal\n\nSafe patch body.\n");

    /* Pending approval must not be applied. */
    system("mkdir Build\\NxApprovedPatchApplyEngineTestsSandbox\\Knowledge >nul 2>nul");
    system("mkdir Build\\NxApprovedPatchApplyEngineTestsSandbox\\Knowledge\\NCOS >nul 2>nul");
    system("mkdir Build\\NxApprovedPatchApplyEngineTestsSandbox\\Knowledge\\NCOS\\PatchApprovals >nul 2>nul");
    write_file("Build/NxApprovedPatchApplyEngineTestsSandbox/Knowledge/NCOS/PatchApprovals/test_patch.approval.md",
               "# NCOS Patch Approval Record\n\nRun ID: test_patch\nStatus: pending\nReviewer: human_required\nProposal: Build/NxApprovedPatchApplyEngineTestsSandbox/proposal.patch.md\nDecision: waiting_for_human_approval\n");
    expect_true(!NxApprovedPatchApply_Run(root, "test_patch", &r), "pending approval should not apply");
    expect_true(strcmp(r.status, "pending") == 0, "pending status should be reported");

    /* Approved approval should produce application record. */
    write_file("Build/NxApprovedPatchApplyEngineTestsSandbox/Knowledge/NCOS/PatchApprovals/test_patch.approval.md",
               "# NCOS Patch Approval Record\n\nRun ID: test_patch\nStatus: approved\nReviewer: Jorge\nProposal: Build/NxApprovedPatchApplyEngineTestsSandbox/proposal.patch.md\nDecision: approved_for_application\n");
    expect_true(NxApprovedPatchApply_Run(root, "test_patch", &r), "approved patch should stage application");
    expect_true(r.ok == 1, "result should be ok");
    expect_true(r.approved == 1, "result should be approved");
    expect_true(strcmp(r.status, "staged_for_review") == 0, "status should be staged_for_review");
    expect_true(strstr(r.application_path, "test_patch.applied.md") != 0, "application path should include run id");

    expect_true(NxApprovedPatchApply_Status(root, "test_patch", &r), "status should find staged application");
    expect_true(strcmp(r.status, "staged_for_review") == 0, "status should report staged application");

    if (failures) return 1;
    return 0;
}
