#include "Nexiora/Reasoning/NxGroundedReasoning.h"

#include <stdio.h>
#include <string.h>

static void usage(void)
{
    puts("Uso:");
    puts("  nexiora_reason_grounded ask <evidence.nxevidence> <pregunta>");
}

int main(int argc, char** argv)
{
    NxGroundedAnswer answer;
    NxGroundedReasoningStatus status;
    unsigned int i;
    if (argc != 4 || strcmp(argv[1], "ask") != 0) {
        usage();
        return 2;
    }
    status = NxGroundedReasoning_Ask(argv[2], argv[3], &answer);
    printf("status=%s\n", NxGroundedReasoning_StatusName(status));
    printf("confidence=%u\n", answer.confidence);
    printf("evidence_count=%u\n", answer.evidence_count);
    printf("contradictions=%u\n", answer.contradiction_count);
    printf("answer=%s\n", answer.answer);
    printf("explanation=%s\n", answer.explanation);
    printf("limitations=%s\n", answer.limitations);
    for (i = 0U; i < answer.evidence_count; ++i) {
        printf("evidence_%u_chunk=%u\n", i + 1U, answer.evidence[i].chunk_id);
        printf("evidence_%u_score=%u\n", i + 1U, answer.evidence[i].score);
        printf("evidence_%u_source=%s\n", i + 1U, answer.evidence[i].source);
        printf("evidence_%u_text=%s\n", i + 1U, answer.evidence[i].text);
    }
    return status == NX_GR_OK || status == NX_GR_INSUFFICIENT_EVIDENCE ? 0 : 1;
}
