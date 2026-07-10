#include "Nexiora/Reasoning/NxGroundedReasoning.h"

#include <stdio.h>
#include <string.h>

#define REQUIRE(c, m) do { if (!(c)) { fprintf(stderr, "NxGroundedReasoningTests: FAIL: %s\n", (m)); return 1; } } while (0)

int main(void)
{
    const char* path = "nx_grounded_reasoning_test.nxevidence";
    FILE* f = fopen(path, "wb");
    NxGroundedAnswer answer;
    NxGroundedReasoningStatus status;
    REQUIRE(f != NULL, "cannot create evidence fixture");
    REQUIRE(fputs("chunk=1\nsource=video@00:01:10\ntext=La memoria episodica conserva experiencias personales.\n---\n", f) >= 0, "write failed");
    REQUIRE(fputs("chunk=2\nsource=video@00:02:05\ntext=Las experiencias episodicas incluyen tiempo lugar y contexto.\n---\n", f) >= 0, "write failed");
    REQUIRE(fputs("chunk=3\nsource=video@00:03:00\ntext=La memoria semantica conserva conocimientos generales.\n---\n", f) >= 0, "write failed");
    REQUIRE(fclose(f) == 0, "close failed");

    status = NxGroundedReasoning_Ask(path, "Que conserva la memoria episodica y que contexto incluye", &answer);
    REQUIRE(status == NX_GR_OK, "grounded query failed");
    REQUIRE(answer.evidence_count >= 2U, "multiple evidence was not fused");
    REQUIRE(strstr(answer.answer, "experiencias personales") != NULL, "first evidence absent");
    REQUIRE(strstr(answer.answer, "tiempo lugar y contexto") != NULL, "second evidence absent");
    REQUIRE(answer.confidence > 0U, "confidence missing");

    status = NxGroundedReasoning_Ask(path, "Que dice sobre combustion nuclear", &answer);
    REQUIRE(status == NX_GR_INSUFFICIENT_EVIDENCE, "unsupported query was not rejected");
    REQUIRE(remove(path) == 0, "cleanup failed");
    puts("NxGroundedReasoningTests: PASS");
    return 0;
}
