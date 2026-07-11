#include "Nexiora/Reasoning/NxUnifiedCognitiveDialogue.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void nx_ucd_copy(char* dst, size_t cap, const char* src) {
    size_t n;
    if (dst == NULL || cap == 0U) {
        return;
    }
    dst[0] = '\0';
    if (src == NULL) {
        return;
    }
    n = strlen(src);
    if (n >= cap) {
        n = cap - 1U;
    }
    if (n > 0U) {
        memcpy(dst, src, n);
    }
    dst[n] = '\0';
}

static int nx_ucd_read_session(const char* session_path,
                               char* evidence,
                               size_t evidence_cap,
                               char* subject,
                               size_t subject_cap,
                               unsigned int* turns) {
    FILE* file;
    char line[4096];
    if (session_path == NULL || evidence == NULL || subject == NULL || turns == NULL) {
        return 0;
    }
    evidence[0] = '\0';
    subject[0] = '\0';
    *turns = 0U;
    file = fopen(session_path, "rb");
    if (file == NULL) {
        return 0;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        while (len > 0U && (line[len - 1U] == '\n' || line[len - 1U] == '\r')) {
            line[--len] = '\0';
        }
        if (strncmp(line, "evidence=", 9U) == 0) {
            nx_ucd_copy(evidence, evidence_cap, line + 9U);
        } else if (strncmp(line, "subject=", 8U) == 0) {
            nx_ucd_copy(subject, subject_cap, line + 8U);
        } else if (strncmp(line, "turns=", 6U) == 0) {
            unsigned int parsed = 0U;
            if (sscanf(line + 6U, "%u", &parsed) == 1) {
                *turns = parsed;
            }
        }
    }
    (void)fclose(file);
    return evidence[0] != '\0' && subject[0] != '\0';
}

static void nx_ucd_sanitize_id(const char* text, char* out, size_t cap) {
    size_t i;
    size_t w = 0U;
    if (out == NULL || cap == 0U) {
        return;
    }
    out[0] = '\0';
    if (text == NULL) {
        return;
    }
    for (i = 0U; text[i] != '\0' && w + 1U < cap; ++i) {
        unsigned char c = (unsigned char)text[i];
        if (isalnum(c) != 0) {
            out[w++] = (char)tolower(c);
        } else if (w > 0U && out[w - 1U] != '_') {
            out[w++] = '_';
        }
    }
    while (w > 0U && out[w - 1U] == '_') {
        --w;
    }
    out[w] = '\0';
}

static int nx_ucd_build_gap_path(const char* session_path,
                                 unsigned int turn,
                                 char* out,
                                 size_t cap) {
    char id[128];
    int written;
    nx_ucd_sanitize_id(session_path, id, sizeof(id));
    if (id[0] == '\0') {
        nx_ucd_copy(id, sizeof(id), "conversation");
    }
    written = snprintf(out, cap, "Knowledge/ResearchGaps/%s_turn_%u.nxgap", id, turn);
    return written >= 0 && (size_t)written < cap;
}

static NxUcdStatus nx_ucd_open_gap(const char* evidence,
                                   const char* subject,
                                   const char* question,
                                   unsigned int turn,
                                   NxUcdResult* result) {
    NxKnowledgeGapPlan plan;
    NxKnowledgeGapStatus gap_status;
    if (!nx_ucd_build_gap_path(question, turn, result->gap_plan_path, sizeof(result->gap_plan_path))) {
        return NX_UCD_IO_ERROR;
    }
    gap_status = NxKnowledgeGapResearch_Assess(
        evidence, subject, question, result->gap_plan_path, &plan);
    if (gap_status == NX_KGR_GAP_OPENED) {
        result->confidence = plan.confidence;
        nx_ucd_copy(result->explanation, sizeof(result->explanation), plan.gap_reason);
        nx_ucd_copy(result->limitations, sizeof(result->limitations), plan.success_criteria);
        nx_ucd_copy(result->gap_plan_path, sizeof(result->gap_plan_path), plan.plan_path);
        nx_ucd_copy(result->answer, sizeof(result->answer),
                    "No existe evidencia suficiente para responder sin especular. Nexiora abrió un plan de investigación auditable.");
        return NX_UCD_GAP_OPENED;
    }
    if (gap_status == NX_KGR_SUFFICIENT) {
        return NX_UCD_OK;
    }
    return NX_UCD_REASONING_ERROR;
}

NxUcdStatus NxUnifiedCognitiveDialogue_Create(const char* session_path,
                                              const char* evidence_path,
                                              const char* initial_subject) {
    NxCcStatus status = NxConversationalContext_Create(session_path, evidence_path, initial_subject);
    if (status == NX_CC_OK) {
        return NX_UCD_OK;
    }
    if (status == NX_CC_INVALID_ARGUMENT) {
        return NX_UCD_INVALID_ARGUMENT;
    }
    return NX_UCD_IO_ERROR;
}

NxUcdStatus NxUnifiedCognitiveDialogue_Ask(const char* session_path,
                                           const char* question,
                                           NxUcdResult* out_result) {
    NxCcTurnResult turn;
    char evidence[NX_UCD_MAX_PATH];
    char subject[NX_UCD_MAX_TEXT];
    unsigned int prior_turns = 0U;
    NxCcStatus cc_status;
    NxUcdResult result;
    if (session_path == NULL || question == NULL || out_result == NULL || question[0] == '\0') {
        return NX_UCD_INVALID_ARGUMENT;
    }
    memset(&result, 0, sizeof(result));
    result.mode = NX_UCD_MODE_ANSWER;
    if (!nx_ucd_read_session(session_path, evidence, sizeof(evidence), subject, sizeof(subject), &prior_turns)) {
        return NX_UCD_IO_ERROR;
    }
    cc_status = NxConversationalContext_Ask(session_path, question, &turn);
    if (cc_status != NX_CC_OK) {
        return NX_UCD_REASONING_ERROR;
    }
    result.turn_index = turn.turn_index;
    result.confidence = turn.grounded.confidence;
    result.evidence_count = turn.grounded.evidence_count;
    result.contradiction_count = turn.grounded.contradiction_count;
    nx_ucd_copy(result.active_subject, sizeof(result.active_subject), turn.active_subject);
    nx_ucd_copy(result.resolved_input, sizeof(result.resolved_input), turn.resolved_question);
    nx_ucd_copy(result.answer, sizeof(result.answer), turn.grounded.answer);
    nx_ucd_copy(result.explanation, sizeof(result.explanation), turn.grounded.explanation);
    nx_ucd_copy(result.limitations, sizeof(result.limitations), turn.grounded.limitations);
    if (turn.grounded.status == NX_GR_INSUFFICIENT_EVIDENCE || turn.grounded.evidence_count == 0U) {
        result.status = nx_ucd_open_gap(evidence, subject, turn.resolved_question,
                                        prior_turns + 1U, &result);
    } else {
        result.status = NX_UCD_OK;
    }
    *out_result = result;
    return result.status;
}

NxUcdStatus NxUnifiedCognitiveDialogue_EvaluateClaim(const char* session_path,
                                                      const char* claim,
                                                      NxUcdResult* out_result) {
    char evidence[NX_UCD_MAX_PATH];
    char subject[NX_UCD_MAX_TEXT];
    unsigned int turns = 0U;
    NxChResult analysis;
    NxChStatus ch_status;
    NxUcdResult result;
    if (session_path == NULL || claim == NULL || out_result == NULL || claim[0] == '\0') {
        return NX_UCD_INVALID_ARGUMENT;
    }
    memset(&result, 0, sizeof(result));
    result.mode = NX_UCD_MODE_CLAIM;
    if (!nx_ucd_read_session(session_path, evidence, sizeof(evidence), subject, sizeof(subject), &turns)) {
        return NX_UCD_IO_ERROR;
    }
    ch_status = NxContradictionHypothesis_Analyze(evidence, claim, &analysis);
    result.turn_index = turns;
    result.confidence = analysis.confidence;
    result.evidence_count = analysis.evidence_count;
    result.contradiction_count = analysis.contradiction_count;
    nx_ucd_copy(result.active_subject, sizeof(result.active_subject), subject);
    nx_ucd_copy(result.resolved_input, sizeof(result.resolved_input), claim);
    nx_ucd_copy(result.answer, sizeof(result.answer), analysis.conclusion);
    nx_ucd_copy(result.explanation, sizeof(result.explanation), analysis.explanation);
    nx_ucd_copy(result.limitations, sizeof(result.limitations), analysis.missing_evidence);
    if (ch_status == NX_CH_INSUFFICIENT_EVIDENCE) {
        result.status = nx_ucd_open_gap(evidence, subject, claim, turns + 1U, &result);
    } else if (ch_status == NX_CH_CONTRADICTORY_EVIDENCE) {
        result.status = NX_UCD_CONTRADICTORY;
    } else if (ch_status == NX_CH_OK) {
        result.status = NX_UCD_OK;
    } else {
        result.status = NX_UCD_REASONING_ERROR;
    }
    *out_result = result;
    return result.status;
}

const char* NxUnifiedCognitiveDialogue_StatusName(NxUcdStatus status) {
    switch (status) {
        case NX_UCD_OK: return "OK";
        case NX_UCD_GAP_OPENED: return "GAP_OPENED";
        case NX_UCD_CONTRADICTORY: return "CONTRADICTORY_EVIDENCE";
        case NX_UCD_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_UCD_IO_ERROR: return "IO_ERROR";
        case NX_UCD_REASONING_ERROR: return "REASONING_ERROR";
        default: return "UNKNOWN";
    }
}
