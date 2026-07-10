#include "Nexiora/Reasoning/NxContradictionHypothesis.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NX_CH_MAX_CHUNKS 256U
#define NX_CH_MAX_TERMS 48U
#define NX_CH_MAX_TERM 64U
#define NX_CH_LINE 4096U

typedef struct NxChChunk {
    unsigned int id;
    char source[NX_CH_MAX_PATH];
    char text[NX_CH_MAX_TEXT];
    unsigned int relevance;
    NxChStance stance;
} NxChChunk;

static int nx_ch_copy(char* dst, size_t size, const char* src)
{
    size_t n;
    if (dst == NULL || src == NULL || size == 0U) return 0;
    n = strlen(src);
    if (n >= size) return 0;
    memcpy(dst, src, n + 1U);
    return 1;
}

static void nx_ch_trim(char* text)
{
    size_t start = 0U;
    size_t end;
    if (text == NULL) return;
    end = strlen(text);
    while (start < end && isspace((unsigned char)text[start]) != 0) ++start;
    while (end > start && isspace((unsigned char)text[end - 1U]) != 0) --end;
    if (start > 0U) memmove(text, text + start, end - start);
    text[end - start] = '\0';
}

static int nx_ch_contains_ci(const char* text, const char* needle)
{
    size_t i;
    size_t n;
    if (text == NULL || needle == NULL) return 0;
    n = strlen(needle);
    if (n == 0U) return 0;
    for (i = 0U; text[i] != '\0'; ++i) {
        size_t j = 0U;
        while (j < n && text[i + j] != '\0' &&
               tolower((unsigned char)text[i + j]) == tolower((unsigned char)needle[j])) ++j;
        if (j == n) return 1;
    }
    return 0;
}

static int nx_ch_stopword(const char* word)
{
    static const char* const stop[] = {
        "que", "como", "cual", "para", "por", "con", "sin", "del", "las", "los", "una", "uno",
        "esta", "este", "esto", "sobre", "entre", "desde", "hasta", "the", "what", "which", "with",
        "from", "that", "this", "about", "and", "for", "es", "son", "ser"
    };
    size_t i;
    for (i = 0U; i < sizeof(stop) / sizeof(stop[0]); ++i) if (strcmp(word, stop[i]) == 0) return 1;
    return 0;
}

static unsigned int nx_ch_terms(const char* claim, char terms[][NX_CH_MAX_TERM])
{
    unsigned int count = 0U;
    char word[NX_CH_MAX_TERM];
    size_t used = 0U;
    size_t i;
    size_t len;
    if (claim == NULL) return 0U;
    len = strlen(claim);
    for (i = 0U; i <= len; ++i) {
        unsigned char c = (unsigned char)claim[i];
        if (isalnum(c) != 0 && used + 1U < sizeof(word)) {
            word[used++] = (char)tolower(c);
        } else if (used > 0U) {
            unsigned int duplicate = 0U;
            unsigned int j;
            word[used] = '\0';
            if (used >= 3U && nx_ch_stopword(word) == 0) {
                for (j = 0U; j < count; ++j) if (strcmp(terms[j], word) == 0) duplicate = 1U;
                if (duplicate == 0U && count < NX_CH_MAX_TERMS) {
                    (void)nx_ch_copy(terms[count], NX_CH_MAX_TERM, word);
                    ++count;
                }
            }
            used = 0U;
        }
    }
    return count;
}

static unsigned int nx_ch_relevance(const char* text, char terms[][NX_CH_MAX_TERM], unsigned int count)
{
    unsigned int score = 0U;
    unsigned int i;
    for (i = 0U; i < count; ++i) if (nx_ch_contains_ci(text, terms[i]) != 0) score += 20U;
    return score > 100U ? 100U : score;
}

static NxChStance nx_ch_stance(const char* text)
{
    static const char* const negations[] = {
        " no ", " nunca ", " falso ", " incorrecto ", " contradice ", " no depende ", " no es ",
        "not ", "never ", "false ", "incorrect ", "does not ", "is not "
    };
    size_t i;
    char padded[NX_CH_MAX_TEXT + 3U];
    int written = snprintf(padded, sizeof(padded), " %s ", text == NULL ? "" : text);
    if (written < 0 || (size_t)written >= sizeof(padded)) return NX_CH_STANCE_NEUTRAL;
    for (i = 0U; i < sizeof(negations) / sizeof(negations[0]); ++i) {
        if (nx_ch_contains_ci(padded, negations[i]) != 0) return NX_CH_STANCE_OPPOSE;
    }
    return NX_CH_STANCE_SUPPORT;
}

static int nx_ch_parse(const char* path, NxChChunk* chunks, unsigned int* out_count)
{
    FILE* file;
    char line[NX_CH_LINE];
    NxChChunk current;
    unsigned int count = 0U;
    int active = 0;
    if (path == NULL || chunks == NULL || out_count == NULL) return 0;
    file = fopen(path, "rb");
    if (file == NULL) return 0;
    memset(&current, 0, sizeof(current));
    while (fgets(line, sizeof(line), file) != NULL) {
        nx_ch_trim(line);
        if (strncmp(line, "chunk=", 6U) == 0) {
            if (active != 0 && count < NX_CH_MAX_CHUNKS) chunks[count++] = current;
            memset(&current, 0, sizeof(current));
            current.id = (unsigned int)strtoul(line + 6, NULL, 10);
            active = 1;
        } else if (strncmp(line, "source=", 7U) == 0) {
            if (nx_ch_copy(current.source, sizeof(current.source), line + 7) == 0) { fclose(file); return 0; }
        } else if (strncmp(line, "text=", 5U) == 0) {
            if (nx_ch_copy(current.text, sizeof(current.text), line + 5) == 0) { fclose(file); return 0; }
        }
    }
    if (active != 0 && count < NX_CH_MAX_CHUNKS) chunks[count++] = current;
    if (fclose(file) != 0) return 0;
    *out_count = count;
    return count > 0U ? 1 : 0;
}

static void nx_ch_sort(NxChChunk* chunks, unsigned int count)
{
    unsigned int i;
    unsigned int j;
    for (i = 0U; i < count; ++i) {
        for (j = i + 1U; j < count; ++j) {
            if (chunks[j].relevance > chunks[i].relevance) {
                NxChChunk tmp = chunks[i]; chunks[i] = chunks[j]; chunks[j] = tmp;
            }
        }
    }
}

NxChStatus NxContradictionHypothesis_Analyze(const char* evidence_path, const char* claim, NxChResult* out_result)
{
    NxChChunk chunks[NX_CH_MAX_CHUNKS];
    char terms[NX_CH_MAX_TERMS][NX_CH_MAX_TERM];
    unsigned int chunk_count = 0U;
    unsigned int term_count;
    unsigned int i;
    unsigned int selected = 0U;
    unsigned int support_score = 0U;
    unsigned int oppose_score = 0U;
    unsigned int support_count = 0U;
    unsigned int oppose_count = 0U;
    if (evidence_path == NULL || claim == NULL || out_result == NULL || claim[0] == '\0') return NX_CH_INVALID_ARGUMENT;
    memset(out_result, 0, sizeof(*out_result));
    memset(chunks, 0, sizeof(chunks));
    term_count = nx_ch_terms(claim, terms);
    if (term_count == 0U) {
        out_result->status = NX_CH_INSUFFICIENT_EVIDENCE;
        (void)nx_ch_copy(out_result->missing_evidence, sizeof(out_result->missing_evidence), "La afirmación no contiene conceptos analizables.");
        return out_result->status;
    }
    if (nx_ch_parse(evidence_path, chunks, &chunk_count) == 0) {
        out_result->status = NX_CH_SOURCE_NOT_FOUND;
        (void)nx_ch_copy(out_result->missing_evidence, sizeof(out_result->missing_evidence), "No fue posible leer el archivo de evidencia.");
        return out_result->status;
    }
    for (i = 0U; i < chunk_count; ++i) {
        chunks[i].relevance = nx_ch_relevance(chunks[i].text, terms, term_count);
        chunks[i].stance = nx_ch_stance(chunks[i].text);
    }
    nx_ch_sort(chunks, chunk_count);
    for (i = 0U; i < chunk_count && selected < NX_CH_MAX_EVIDENCE; ++i) {
        NxChEvidence* dst;
        if (chunks[i].relevance < 20U) break;
        dst = &out_result->evidence[selected];
        dst->chunk_id = chunks[i].id;
        dst->relevance = chunks[i].relevance;
        dst->stance = chunks[i].stance;
        (void)nx_ch_copy(dst->source, sizeof(dst->source), chunks[i].source);
        (void)nx_ch_copy(dst->text, sizeof(dst->text), chunks[i].text);
        if (chunks[i].stance == NX_CH_STANCE_OPPOSE) { oppose_score += chunks[i].relevance; ++oppose_count; }
        else { support_score += chunks[i].relevance; ++support_count; }
        ++selected;
    }
    if (selected == 0U) {
        out_result->status = NX_CH_INSUFFICIENT_EVIDENCE;
        (void)nx_ch_copy(out_result->conclusion, sizeof(out_result->conclusion), "No existe evidencia relevante suficiente para evaluar la afirmación sin especular.");
        (void)nx_ch_copy(out_result->missing_evidence, sizeof(out_result->missing_evidence), "Se requieren fuentes que mencionen directamente los conceptos principales de la afirmación.");
        return out_result->status;
    }
    out_result->evidence_count = selected;
    out_result->hypothesis_count = 2U;
    (void)nx_ch_copy(out_result->hypotheses[0].statement, sizeof(out_result->hypotheses[0].statement), claim);
    out_result->hypotheses[0].support_score = support_score;
    out_result->hypotheses[0].opposition_score = oppose_score;
    out_result->hypotheses[0].support_count = support_count;
    out_result->hypotheses[0].opposition_count = oppose_count;
    (void)nx_ch_copy(out_result->hypotheses[1].statement, sizeof(out_result->hypotheses[1].statement), "Hipótesis alternativa: la afirmación requiere matices o condiciones adicionales.");
    out_result->hypotheses[1].support_score = oppose_score;
    out_result->hypotheses[1].opposition_score = support_score;
    out_result->hypotheses[1].support_count = oppose_count;
    out_result->hypotheses[1].opposition_count = support_count;
    if (support_score + oppose_score > 0U) {
        out_result->hypotheses[0].confidence = (support_score * 100U) / (support_score + oppose_score);
        out_result->hypotheses[1].confidence = (oppose_score * 100U) / (support_score + oppose_score);
    }
    out_result->confidence = out_result->hypotheses[0].confidence;
    if (support_count > 0U && oppose_count > 0U) {
        out_result->status = NX_CH_CONTRADICTORY_EVIDENCE;
        out_result->contradiction_count = support_count < oppose_count ? support_count : oppose_count;
        (void)nx_ch_copy(out_result->conclusion, sizeof(out_result->conclusion), "La evidencia es contradictoria: existen fuentes que apoyan y fuentes que se oponen a la afirmación.");
        (void)nx_ch_copy(out_result->explanation, sizeof(out_result->explanation), "Nexiora mantuvo ambas hipótesis activas y calculó su soporte relativo; no seleccionó una conclusión absoluta.");
        (void)nx_ch_copy(out_result->missing_evidence, sizeof(out_result->missing_evidence), "Se requieren fuentes independientes, condiciones comparables o mediciones que permitan resolver la contradicción.");
    } else if (support_count > 0U) {
        out_result->status = NX_CH_OK;
        (void)nx_ch_copy(out_result->conclusion, sizeof(out_result->conclusion), "La evidencia disponible apoya la afirmación evaluada.");
        (void)nx_ch_copy(out_result->explanation, sizeof(out_result->explanation), "Los fragmentos relevantes presentan una postura compatible y no se detectó oposición explícita.");
        (void)nx_ch_copy(out_result->missing_evidence, sizeof(out_result->missing_evidence), "La confianza aumentaría con más fuentes independientes y evidencia cuantitativa.");
    } else {
        out_result->status = NX_CH_OK;
        out_result->confidence = out_result->hypotheses[1].confidence;
        (void)nx_ch_copy(out_result->conclusion, sizeof(out_result->conclusion), "La evidencia disponible se opone a la afirmación evaluada.");
        (void)nx_ch_copy(out_result->explanation, sizeof(out_result->explanation), "Los fragmentos relevantes contienen negaciones o refutaciones explícitas.");
        (void)nx_ch_copy(out_result->missing_evidence, sizeof(out_result->missing_evidence), "Se requieren fuentes de apoyo para reconsiderar la afirmación.");
    }
    return out_result->status;
}

const char* NxContradictionHypothesis_StatusName(NxChStatus status)
{
    switch (status) {
        case NX_CH_OK: return "OK";
        case NX_CH_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_CH_SOURCE_NOT_FOUND: return "SOURCE_NOT_FOUND";
        case NX_CH_PARSE_ERROR: return "PARSE_ERROR";
        case NX_CH_INSUFFICIENT_EVIDENCE: return "INSUFFICIENT_EVIDENCE";
        case NX_CH_CONTRADICTORY_EVIDENCE: return "CONTRADICTORY_EVIDENCE";
        default: return "UNKNOWN";
    }
}

const char* NxContradictionHypothesis_StanceName(NxChStance stance)
{
    switch (stance) {
        case NX_CH_STANCE_SUPPORT: return "SUPPORT";
        case NX_CH_STANCE_OPPOSE: return "OPPOSE";
        case NX_CH_STANCE_NEUTRAL: return "NEUTRAL";
        default: return "UNKNOWN";
    }
}
