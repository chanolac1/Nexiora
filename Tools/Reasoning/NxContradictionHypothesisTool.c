#include "Nexiora/Reasoning/NxContradictionHypothesis.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    NxChResult result;
    unsigned int i;
    NxChStatus status;
    if (argc < 4 || strcmp(argv[1], "analyze") != 0) {
        fprintf(stderr, "Usage: nexiora_hypothesis analyze <evidence.nxevidence> <claim>\n");
        return 2;
    }
    status = NxContradictionHypothesis_Analyze(argv[2], argv[3], &result);
    printf("status=%s\nconfidence=%u\ncontradictions=%u\nevidence_count=%u\nhypothesis_count=%u\n",
           NxContradictionHypothesis_StatusName(status), result.confidence, result.contradiction_count,
           result.evidence_count, result.hypothesis_count);
    printf("conclusion=%s\nexplanation=%s\nmissing_evidence=%s\n", result.conclusion, result.explanation, result.missing_evidence);
    for (i = 0U; i < result.hypothesis_count; ++i) {
        printf("hypothesis_%u=%s\nhypothesis_%u_support=%u\nhypothesis_%u_opposition=%u\nhypothesis_%u_confidence=%u\n",
               i + 1U, result.hypotheses[i].statement, i + 1U, result.hypotheses[i].support_score,
               i + 1U, result.hypotheses[i].opposition_score, i + 1U, result.hypotheses[i].confidence);
    }
    for (i = 0U; i < result.evidence_count; ++i) {
        printf("evidence_%u_stance=%s\nevidence_%u_source=%s\nevidence_%u_text=%s\n",
               i + 1U, NxContradictionHypothesis_StanceName(result.evidence[i].stance),
               i + 1U, result.evidence[i].source, i + 1U, result.evidence[i].text);
    }
    return status == NX_CH_OK || status == NX_CH_CONTRADICTORY_EVIDENCE || status == NX_CH_INSUFFICIENT_EVIDENCE ? 0 : 1;
}
