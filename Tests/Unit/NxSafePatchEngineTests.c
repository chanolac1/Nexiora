#include "Nexiora/NCOS/NxSafePatchEngine.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    const char* root = "Build/NxSafePatchEngineTestsSandbox";
    const char* analysis = "Build/NxSafePatchEngineTestsSandbox/sample.analysis.md";
    NxSafePatchResult result;
    FILE* f;

    system("if exist Build\\NxSafePatchEngineTestsSandbox rmdir /s /q Build\\NxSafePatchEngineTestsSandbox >nul 2>nul");
    system("if not exist Build mkdir Build >nul 2>nul");
    system("mkdir Build\\NxSafePatchEngineTestsSandbox >nul 2>nul");

    f = fopen(analysis, "wb");
    check(f != NULL, "should create sample analysis");
    if (f) {
        fprintf(f, "FAILED: CMakeFiles/demo.obj\n");
        fprintf(f, "D:/Nexiora/Source/demo.c:10:5: error: expected ';' before 'return'\n");
        fprintf(f, "D:/Nexiora/Source/demo.c:11:1: warning: unused variable 'x'\n");
        fprintf(f, "The following tests FAILED:\n");
        fclose(f);
    }

    check(NxSafePatch_CreateProposal(root, "Compile Fix 001", analysis, &result) == 1, "create proposal should succeed");
    check(result.ok == 1, "result should be ok");
    check(strcmp(result.run_id, "compile_fix_001") == 0, "run id should be normalized");
    check(result.proposed_changes >= 2, "should propose actionable changes");
    check(result.requires_human_approval == 1, "should require human approval");
    check(strlen(result.proposal_path) > 0, "should expose proposal path");
    check(file_contains(result.proposal_path, "NCOS Safe Patch Proposal"), "proposal should have title");
    check(file_contains(result.proposal_path, "waiting_for_human_approval"), "proposal should include decision gate");
    check(file_contains(result.proposal_path, "missing semicolon"), "proposal should include syntax action");

    return failures == 0 ? 0 : 1;
}
