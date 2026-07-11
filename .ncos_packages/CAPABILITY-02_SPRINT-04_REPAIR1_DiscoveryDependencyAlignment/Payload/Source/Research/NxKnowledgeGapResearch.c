#include "Nexiora/Research/NxKnowledgeGapResearch.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_KGR_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_KGR_MKDIR(path) mkdir((path), 0777)
#endif

static int nx_kgr_copy(char* dst, size_t size, const char* src)
{
    size_t n;
    if (dst == NULL || src == NULL || size == 0U) return 0;
    n = strlen(src);
    if (n >= size) return 0;
    memcpy(dst, src, n + 1U);
    return 1;
}


static int nx_kgr_join(char* dst, size_t size, const char* left, const char* right)
{
    size_t a;
    size_t b;
    if (dst == NULL || left == NULL || right == NULL || size == 0U) return 0;
    a = strlen(left);
    b = strlen(right);
    if (a + b >= size) return 0;
    memcpy(dst, left, a);
    memcpy(dst + a, right, b + 1U);
    return 1;
}

static int nx_kgr_mkdirs_for_file(const char* file_path)
{
    char path[NX_KGR_MAX_PATH];
    size_t i;
    if (file_path == NULL || nx_kgr_copy(path, sizeof(path), file_path) == 0) return 0;
    for (i = 1U; path[i] != '\0'; ++i) {
        if (path[i] == '/' || path[i] == '\\') {
            char saved = path[i];
            path[i] = '\0';
            if (!(i == 2U && path[1] == ':') && NX_KGR_MKDIR(path) != 0 && errno != EEXIST) return 0;
            path[i] = saved;
        }
    }
    return 1;
}

static void nx_kgr_slug_terms(const char* question, char* output, size_t size)
{
    size_t i;
    size_t w = 0U;
    int separator = 0;
    if (output == NULL || size == 0U) return;
    output[0] = '\0';
    if (question == NULL) return;
    for (i = 0U; question[i] != '\0' && w + 1U < size; ++i) {
        unsigned char ch = (unsigned char)question[i];
        if (isalnum(ch) != 0) {
            output[w++] = (char)tolower(ch);
            separator = 1;
        } else if (separator != 0 && w + 1U < size) {
            output[w++] = ' ';
            separator = 0;
        }
    }
    while (w > 0U && output[w - 1U] == ' ') --w;
    output[w] = '\0';
}

static int nx_kgr_write(const NxKnowledgeGapPlan* plan)
{
    FILE* file;
    unsigned int i;
    if (plan == NULL || nx_kgr_mkdirs_for_file(plan->plan_path) == 0) return 0;
    file = fopen(plan->plan_path, "wb");
    if (file == NULL) return 0;
    if (fprintf(file,
        "nxgap/1\nstatus=OPEN\nsubject=%s\nquestion=%s\nreason=%s\nconfidence=%u\nallowed_sources=%s\nsuccess_criteria=%s\nquery_count=%u\n",
        plan->subject, plan->original_question, plan->gap_reason, plan->confidence,
        plan->allowed_sources, plan->success_criteria, plan->query_count) < 0) {
        (void)fclose(file);
        return 0;
    }
    for (i = 0U; i < plan->query_count; ++i) {
        if (fprintf(file, "query.%u=%s\n", i + 1U, plan->queries[i]) < 0) {
            (void)fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0 ? 1 : 0;
}

NxKnowledgeGapStatus NxKnowledgeGapResearch_Assess(
    const char* evidence_path,
    const char* subject,
    const char* question,
    const char* plan_path,
    NxKnowledgeGapPlan* out_plan)
{
    NxGroundedAnswer answer;
    NxGroundedReasoningStatus reasoning;
    char normalized[NX_KGR_MAX_QUERY];
    if (evidence_path == NULL || subject == NULL || question == NULL || plan_path == NULL ||
        out_plan == NULL || evidence_path[0] == '\0' || subject[0] == '\0' ||
        question[0] == '\0' || plan_path[0] == '\0') return NX_KGR_INVALID_ARGUMENT;
    memset(out_plan, 0, sizeof(*out_plan));
    if (nx_kgr_copy(out_plan->subject, sizeof(out_plan->subject), subject) == 0 ||
        nx_kgr_copy(out_plan->original_question, sizeof(out_plan->original_question), question) == 0 ||
        nx_kgr_copy(out_plan->plan_path, sizeof(out_plan->plan_path), plan_path) == 0) {
        out_plan->status = NX_KGR_INVALID_ARGUMENT;
        return out_plan->status;
    }
    reasoning = NxGroundedReasoning_Ask(evidence_path, question, &answer);
    out_plan->confidence = answer.confidence;
    if (reasoning == NX_GR_OK && answer.confidence >= 40U && answer.evidence_count > 0U) {
        out_plan->status = NX_KGR_SUFFICIENT;
        (void)nx_kgr_copy(out_plan->gap_reason, sizeof(out_plan->gap_reason),
                          "La evidencia existente permite responder sin abrir una investigación.");
        return out_plan->status;
    }
    if (reasoning != NX_GR_INSUFFICIENT_EVIDENCE && reasoning != NX_GR_OK) {
        out_plan->status = NX_KGR_REASONING_ERROR;
        (void)nx_kgr_copy(out_plan->gap_reason, sizeof(out_plan->gap_reason),
                          "No fue posible evaluar la evidencia disponible.");
        return out_plan->status;
    }
    nx_kgr_slug_terms(question, normalized, sizeof(normalized));
    out_plan->status = NX_KGR_GAP_OPENED;
    out_plan->query_count = 3U;
    (void)nx_kgr_copy(out_plan->gap_reason, sizeof(out_plan->gap_reason),
                      "La evidencia disponible no alcanza el umbral mínimo de relevancia y confianza.");
    (void)nx_kgr_copy(out_plan->allowed_sources, sizeof(out_plan->allowed_sources),
                      "documentación primaria, repositorios oficiales, artículos técnicos y transcripciones verificables");
    (void)nx_kgr_copy(out_plan->success_criteria, sizeof(out_plan->success_criteria),
                      "Obtener al menos dos evidencias independientes, una fuente primaria y confianza fundamentada >= 60.");
    if (nx_kgr_join(out_plan->queries[0], sizeof(out_plan->queries[0]), normalized, " evidencia primaria") == 0 ||
        nx_kgr_join(out_plan->queries[1], sizeof(out_plan->queries[1]), normalized, " explicación técnica") == 0 ||
        nx_kgr_join(out_plan->queries[2], sizeof(out_plan->queries[2]), normalized, " contradicciones limitaciones") == 0) {
        out_plan->status = NX_KGR_INVALID_ARGUMENT;
        return out_plan->status;
    }
    if (nx_kgr_write(out_plan) == 0) {
        out_plan->status = NX_KGR_IO_ERROR;
        return out_plan->status;
    }
    return out_plan->status;
}

const char* NxKnowledgeGapResearch_StatusName(NxKnowledgeGapStatus status)
{
    switch (status) {
        case NX_KGR_SUFFICIENT: return "SUFFICIENT_EVIDENCE";
        case NX_KGR_GAP_OPENED: return "GAP_OPENED";
        case NX_KGR_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_KGR_IO_ERROR: return "IO_ERROR";
        case NX_KGR_REASONING_ERROR: return "REASONING_ERROR";
        default: return "UNKNOWN";
    }
}
