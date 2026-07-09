#include "Nexiora/NCOS/NxBuildLogAnalyzer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int failures = 0;
static void check(int cond, const char* msg)
{
    if (!cond) { printf("FAIL: %s\n", msg); failures++; }
}

int main(void)
{
    FILE* fp;
    NxBuildLogAnalysis a;
    const char* root = "Build/ncos010_test_sandbox";
    const char* log = "Build/ncos010_test_sandbox/sample_build.log";

#if defined(_WIN32)
    (void)system("if not exist Build mkdir Build >nul 2>nul");
    (void)system("if not exist Build\\ncos010_test_sandbox mkdir Build\\ncos010_test_sandbox >nul 2>nul");
#else
    (void)system("mkdir -p Build/ncos010_test_sandbox");
#endif

    fp = fopen(log, "wb");
    check(fp != NULL, "should create sample log");
    if (!fp) return 1;
    fprintf(fp, "main.c:12: warning: unused variable x\n");
    fprintf(fp, "main.c:18: error: expected ';' before return\n");
    fprintf(fp, "1/1 Test #1: DemoTests ***Failed 0.01 sec\n");
    fclose(fp);

    memset(&a, 0, sizeof(a));
    check(NxBuildLogAnalyzer_AnalyzeFile(root, "demo_run", log, &a) == 1, "analyze should succeed");
    check(a.success == 0, "analysis should fail when errors exist");
    check(a.error_count >= 1, "should count errors");
    check(a.warning_count >= 1, "should count warnings");
    check(a.finding_count >= 2, "should collect findings");
    check(strstr(a.report_path, "demo_run") != NULL, "report path should include run id");

    fp = fopen(a.report_path, "rb");
    check(fp != NULL, "should write analysis report");
    if (fp) fclose(fp);

    return failures ? 1 : 0;
}
