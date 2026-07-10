#include "Nexiora/Reasoning/NxContradictionHypothesis.h"
#include <stdio.h>
#include <string.h>

#define REQUIRE(c,m) do { if (!(c)) { fprintf(stderr, "NxContradictionHypothesisTests: FAIL: %s\n", (m)); return 1; } } while (0)

int main(void)
{
    NxChResult result;
    NxChStatus status;
    status = NxContradictionHypothesis_Analyze("Samples/Reasoning/consolidation_conflict.nxevidence",
        "La consolidacion de memoria depende principalmente del sueno", &result);
    REQUIRE(status == NX_CH_CONTRADICTORY_EVIDENCE, "contradiction not detected");
    REQUIRE(result.evidence_count >= 2U, "multiple evidence not selected");
    REQUIRE(result.contradiction_count >= 1U, "contradiction count missing");
    REQUIRE(result.hypothesis_count == 2U, "hypotheses not generated");
    REQUIRE(result.hypotheses[0].support_count > 0U, "support evidence missing");
    REQUIRE(result.hypotheses[0].opposition_count > 0U, "opposition evidence missing");
    REQUIRE(strstr(result.missing_evidence, "fuentes") != NULL, "knowledge gap not explained");

    status = NxContradictionHypothesis_Analyze("Samples/Reasoning/consolidation_conflict.nxevidence",
        "Los motores nucleares usan helio liquido", &result);
    REQUIRE(status == NX_CH_INSUFFICIENT_EVIDENCE, "unsupported claim was not rejected");
    REQUIRE(result.evidence_count == 0U, "unsupported claim selected evidence");
    puts("NxContradictionHypothesisTests: PASS");
    return 0;
}
