#include "Nexiora/Research/NxKnowledgeAcquisition.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char* message)
{
    if (!condition)
    {
        printf("FAIL: %s\n", message);
        return 1;
    }
    return 0;
}

int main(void)
{
    NxKnowledgeAcquisitionPlan plan;
    char path[260];
    int failures = 0;

    failures += expect(
        NxKnowledgeAcquisition_BuildPlan("SQLite", &plan) == NX_KA_STATUS_OK,
        "SQLite plan should be created");
    failures += expect(strcmp(plan.kind, "software") == 0, "SQLite should be classified as software");
    failures += expect(plan.source_count >= 3, "SQLite plan should include several sources");
    failures += expect(plan.step_count >= 5, "Plan should include research steps");
    failures += expect(plan.expected_confidence >= 80, "Plan should include expected confidence");

    failures += expect(
        NxKnowledgeAcquisition_BuildPlan("RFC QUIC", &plan) == NX_KA_STATUS_OK,
        "RFC plan should be created");
    failures += expect(strcmp(plan.kind, "rfc") == 0, "QUIC should be classified as RFC research");
    failures += expect(plan.sources[0].type == NX_KNOWLEDGE_SOURCE_RFC, "RFC should prioritize RFC source");

    failures += expect(
        NxKnowledgeAcquisition_BuildPlan("libro Clean Architecture", &plan) == NX_KA_STATUS_OK,
        "Book plan should be created");
    failures += expect(strcmp(plan.kind, "book") == 0, "Book should be classified as book research");

    failures += expect(
        NxKnowledgeAcquisition_WritePlanMarkdown(&plan, ".", path, sizeof(path)) == NX_KA_STATUS_OK,
        "Markdown plan should be written");
    failures += expect(strstr(path, "plan.md") != NULL, "Markdown path should point to plan.md");

    failures += expect(
        NxKnowledgeAcquisition_WritePlanJson(&plan, ".", path, sizeof(path)) == NX_KA_STATUS_OK,
        "JSON plan should be written");
    failures += expect(strstr(path, "plan.json") != NULL, "JSON path should point to plan.json");

    if (failures != 0)
    {
        return 1;
    }

    return 0;
}
