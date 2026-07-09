#include "Nexiora/NCOS/NxAutoFixProposalEngine.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static int failures = 0;
static void check(int cond, const char* msg)
{
    if (!cond) { printf("FAIL: %s\n", msg); failures++; }
}

int main(void)
{
    FILE* fp;
    NxAutoFixProposalResult p;
    const char* root = "Build/ncos011_test_sandbox_engine";
    const char* report = "Build/ncos011_test_sandbox_engine/analysis.md";

    NX_MKDIR("Build");
    NX_MKDIR(root);

    fp = fopen(report, "wb");
    check(fp != NULL, "should create input analysis report");
    if (!fp) return 1;
    fprintf(fp, "# Build analysis\n");
    fprintf(fp, "demo.c:10:5: error: expected ';' before 'return'\n");
    fprintf(fp, "demo.c:11:1: warning: unused variable 'x'\n");
    fprintf(fp, "The following tests FAILED:\n");
    fprintf(fp, "  12 - DemoTests (Failed)\n");
    fclose(fp);

    check(NxAutoFixProposalEngine_CreateFromReport(root, "demo_fix", report, &p) == 1, "proposal should succeed");
    check(p.success == 1, "proposal should be successful");
    check(p.suggestion_count >= 3, "proposal should generate several suggestions");
    check(strstr(p.proposal_path, "demo_fix") != NULL, "proposal path should include id");

    fp = fopen(p.proposal_path, "rb");
    check(fp != NULL, "proposal report should be written");
    if (fp) fclose(fp);

    check(strlen(p.suggestions[0].action) > 0, "suggestion should have action text");
    return failures ? 1 : 0;
}
