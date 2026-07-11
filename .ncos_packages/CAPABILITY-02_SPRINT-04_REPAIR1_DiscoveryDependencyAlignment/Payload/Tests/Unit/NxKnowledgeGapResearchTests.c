#include "Nexiora/Research/NxKnowledgeGapResearch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int check(int condition, const char* message)
{
    if (condition != 0) return 1;
    (void)fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    NxKnowledgeGapPlan plan;
    NxKnowledgeGapStatus status;
    FILE* file;
    char content[4096];
    size_t read_count;
    int ok = 1;
    const char* evidence = "Build/nx_gap_test/evidence.nxevidence";
    const char* gap = "Build/nx_gap_test/open_gap.nxgap";
#if defined(_WIN32)
    (void)system("if not exist Build\\nx_gap_test mkdir Build\\nx_gap_test");
#else
    (void)system("mkdir -p Build/nx_gap_test");
#endif
    file = fopen(evidence, "wb");
    if (file == NULL) return 2;
    (void)fputs("nxevidence/1\nchunk=1\nsource=memory@1\ntext=La memoria episodica conserva experiencias personales.\n", file);
    (void)fclose(file);
    status = NxKnowledgeGapResearch_Assess(evidence, "memoria", "Que es un reactor de fusion", gap, &plan);
    ok &= check(status == NX_KGR_GAP_OPENED, "unrelated question should open a gap");
    ok &= check(plan.query_count == 3U, "gap should produce three research queries");
    file = fopen(gap, "rb");
    ok &= check(file != NULL, "gap plan file should exist");
    if (file != NULL) {
        read_count = fread(content, 1U, sizeof(content) - 1U, file);
        content[read_count] = '\0';
        (void)fclose(file);
        ok &= check(strstr(content, "nxgap/1") != NULL, "gap contract marker missing");
        ok &= check(strstr(content, "status=OPEN") != NULL, "gap state missing");
        ok &= check(strstr(content, "query.1=") != NULL, "research query missing");
    }
    status = NxKnowledgeGapResearch_Assess(evidence, "memoria episodica", "Que conserva la memoria episodica", gap, &plan);
    ok &= check(status == NX_KGR_SUFFICIENT, "supported question should not open a gap");
    if (ok == 0) return 1;
    (void)printf("NxKnowledgeGapResearchTests: PASS\n");
    return 0;
}
