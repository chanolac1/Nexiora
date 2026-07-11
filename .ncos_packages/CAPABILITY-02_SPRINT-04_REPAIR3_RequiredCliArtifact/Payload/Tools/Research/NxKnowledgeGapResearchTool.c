#include "Nexiora/Research/NxKnowledgeGapResearch.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    NxKnowledgeGapPlan plan;
    NxKnowledgeGapStatus status;
    unsigned int i;
    if (argc != 6 || argv == NULL || argv[1] == NULL ||
        strcmp(argv[1], "assess") != 0) {
        (void)fprintf(stderr, "Uso: nexiora_gap assess <evidence> <subject> <question> <plan_path>\n");
        return 2;
    }
    status = NxKnowledgeGapResearch_Assess(argv[2], argv[3], argv[4], argv[5], &plan);
    (void)printf("status=%s\nconfidence=%u\nsubject=%s\nquestion=%s\nreason=%s\nplan=%s\nquery_count=%u\n",
                 NxKnowledgeGapResearch_StatusName(status), plan.confidence, plan.subject,
                 plan.original_question, plan.gap_reason, plan.plan_path, plan.query_count);
    for (i = 0U; i < plan.query_count; ++i) (void)printf("query_%u=%s\n", i + 1U, plan.queries[i]);
    if (status == NX_KGR_GAP_OPENED || status == NX_KGR_SUFFICIENT) return 0;
    return 1;
}
