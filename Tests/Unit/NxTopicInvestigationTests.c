#include "Nexiora/Research/NxTopicInvestigation.h"

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
    NxTopicInvestigationResult result;
    NxTopicInvestigationStatus status;
    int failures = 0;

    status = NxTopicInvestigation_Run(".", "SQLite", 0, &result);
    failures += expect(status == NX_TOPIC_INVESTIGATION_OK, "SQLite investigation should run");
    failures += expect(result.concepts_extracted >= 8, "SQLite investigation should extract core concepts");
    failures += expect(result.relations_created >= result.concepts_extracted, "Investigation should create relations");
    failures += expect(result.confidence >= 90, "SQLite confidence should be high for curated offline slice");
    failures += expect(strstr(result.report_path, "report.md") != 0, "Report path should be set");
    failures += expect(strstr(result.memory_path, "memory.jsonl") != 0, "Memory path should be set");

    status = NxTopicInvestigation_Run(".", "", 0, &result);
    failures += expect(status == NX_TOPIC_INVESTIGATION_INVALID_ARGUMENT, "Empty topic should be rejected");

    return failures == 0 ? 0 : 1;
}
