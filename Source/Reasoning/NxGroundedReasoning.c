#include "Nexiora/Reasoning/NxGroundedReasoning.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NX_GR_MAX_CHUNKS 256U
#define NX_GR_MAX_TERMS 64U
#define NX_GR_MAX_TERM 64U
#define NX_GR_LINE 4096U

typedef struct NxGrChunk {
    unsigned int id;
    char source[NX_GR_MAX_PATH];
    char text[NX_GR_MAX_TEXT];
    unsigned int score;
} NxGrChunk;

static int nx_gr_copy(char* dst, size_t dst_size, const char* src)
{
    size_t n;
    if (dst == NULL || src == NULL || dst_size == 0U) return 0;
    n = strlen(src);
    if (n >= dst_size) return 0;
    memcpy(dst, src, n + 1U);
    return 1;
}

static int nx_gr_append(char* dst, size_t dst_size, const char* src)
{
    size_t a;
    size_t b;
    if (dst == NULL || src == NULL || dst_size == 0U) return 0;
    a = strlen(dst);
    b = strlen(src);
    if (a + b >= dst_size) return 0;
    memcpy(dst + a, src, b + 1U);
    return 1;
}

static void nx_gr_trim(char* text)
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

static int nx_gr_stopword(const char* word)
{
    static const char* const words[] = {
        "que", "como", "cual", "cuales", "para", "por", "con", "sin", "del", "las", "los",
        "una", "uno", "unos", "unas", "esta", "este", "esto", "sobre", "entre", "desde", "hasta",
        "the", "what", "which", "with", "from", "that", "this", "into", "about", "and", "for"
    };
    size_t i;
    for (i = 0U; i < sizeof(words) / sizeof(words[0]); ++i) {
        if (strcmp(word, words[i]) == 0) return 1;
    }
    return 0;
}

static unsigned int nx_gr_terms(const char* question, char terms[][NX_GR_MAX_TERM])
{
    unsigned int count = 0U;
    char word[NX_GR_MAX_TERM];
    size_t w = 0U;
    size_t i;
    size_t len;
    if (question == NULL) return 0U;
    len = strlen(question);
    for (i = 0U; i <= len; ++i) {
        unsigned char c = (unsigned char)question[i];
        if (isalnum(c) != 0 && w + 1U < sizeof(word)) {
            word[w++] = (char)tolower(c);
        } else if (w > 0U) {
            unsigned int duplicate = 0U;
            unsigned int j;
            word[w] = '\0';
            if (w >= 3U && nx_gr_stopword(word) == 0) {
                for (j = 0U; j < count; ++j) {
                    if (strcmp(terms[j], word) == 0) duplicate = 1U;
                }
                if (duplicate == 0U && count < NX_GR_MAX_TERMS) {
                    (void)nx_gr_copy(terms[count], NX_GR_MAX_TERM, word);
                    ++count;
                }
            }
            w = 0U;
        }
    }
    return count;
}

static int nx_gr_contains_ci(const char* text, const char* needle)
{
    size_t n;
    size_t i;
    if (text == NULL || needle == NULL) return 0;
    n = strlen(needle);
    if (n == 0U) return 0;
    for (i = 0U; text[i] != '\0'; ++i) {
        size_t j = 0U;
        while (j < n && text[i + j] != '\0' &&
               tolower((unsigned char)text[i + j]) == tolower((unsigned char)needle[j])) {
            ++j;
        }
        if (j == n) return 1;
    }
    return 0;
}

static unsigned int nx_gr_score(const NxGrChunk* chunk, char terms[][NX_GR_MAX_TERM], unsigned int term_count)
{
    unsigned int score = 0U;
    unsigned int i;
    for (i = 0U; i < term_count; ++i) {
        if (nx_gr_contains_ci(chunk->text, terms[i]) != 0) score += 20U;
    }
    if (score > 100U) score = 100U;
    return score;
}

static int nx_gr_parse(const char* path, NxGrChunk* chunks, unsigned int* out_count)
{
    FILE* file;
    char line[NX_GR_LINE];
    NxGrChunk current;
    unsigned int count = 0U;
    int active = 0;
    if (path == NULL || chunks == NULL || out_count == NULL) return 0;
    file = fopen(path, "rb");
    if (file == NULL) return 0;
    memset(&current, 0, sizeof(current));
    while (fgets(line, sizeof(line), file) != NULL) {
        nx_gr_trim(line);
        if (strncmp(line, "chunk=", 6U) == 0) {
            if (active != 0 && count < NX_GR_MAX_CHUNKS) chunks[count++] = current;
            memset(&current, 0, sizeof(current));
            current.id = (unsigned int)strtoul(line + 6, NULL, 10);
            active = 1;
        } else if (strncmp(line, "source=", 7U) == 0) {
            if (nx_gr_copy(current.source, sizeof(current.source), line + 7) == 0) {
                fclose(file);
                return 0;
            }
        } else if (strncmp(line, "text=", 5U) == 0) {
            if (nx_gr_copy(current.text, sizeof(current.text), line + 5) == 0) {
                fclose(file);
                return 0;
            }
        }
    }
    if (active != 0 && count < NX_GR_MAX_CHUNKS) chunks[count++] = current;
    if (fclose(file) != 0) return 0;
    *out_count = count;
    return count > 0U ? 1 : 0;
}

static void nx_gr_sort(NxGrChunk* chunks, unsigned int count)
{
    unsigned int i;
    unsigned int j;
    for (i = 0U; i < count; ++i) {
        for (j = i + 1U; j < count; ++j) {
            if (chunks[j].score > chunks[i].score) {
                NxGrChunk tmp = chunks[i];
                chunks[i] = chunks[j];
                chunks[j] = tmp;
            }
        }
    }
}

static unsigned int nx_gr_detect_contradictions(const NxGrChunk* chunks, unsigned int selected)
{
    unsigned int contradictions = 0U;
    unsigned int i;
    unsigned int j;
    for (i = 0U; i < selected; ++i) {
        for (j = i + 1U; j < selected; ++j) {
            int a_neg = nx_gr_contains_ci(chunks[i].text, " no ");
            int b_neg = nx_gr_contains_ci(chunks[j].text, " no ");
            if (a_neg != b_neg && chunks[i].score == chunks[j].score) ++contradictions;
        }
    }
    return contradictions;
}

NxGroundedReasoningStatus NxGroundedReasoning_Ask(
    const char* evidence_path,
    const char* question,
    NxGroundedAnswer* out_answer)
{
    NxGrChunk chunks[NX_GR_MAX_CHUNKS];
    char terms[NX_GR_MAX_TERMS][NX_GR_MAX_TERM];
    unsigned int chunk_count = 0U;
    unsigned int term_count;
    unsigned int selected = 0U;
    unsigned int i;
    unsigned int score_sum = 0U;
    if (evidence_path == NULL || question == NULL || out_answer == NULL || question[0] == '\0') {
        return NX_GR_INVALID_ARGUMENT;
    }
    memset(out_answer, 0, sizeof(*out_answer));
    memset(chunks, 0, sizeof(chunks));
    term_count = nx_gr_terms(question, terms);
    if (term_count == 0U) {
        out_answer->status = NX_GR_INSUFFICIENT_EVIDENCE;
        (void)nx_gr_copy(out_answer->limitations, sizeof(out_answer->limitations),
                         "La pregunta no contiene conceptos suficientes para buscar evidencia.");
        return out_answer->status;
    }
    if (nx_gr_parse(evidence_path, chunks, &chunk_count) == 0) {
        out_answer->status = NX_GR_SOURCE_NOT_FOUND;
        (void)nx_gr_copy(out_answer->limitations, sizeof(out_answer->limitations),
                         "No fue posible leer evidencia estructurada en la ruta indicada.");
        return out_answer->status;
    }
    for (i = 0U; i < chunk_count; ++i) chunks[i].score = nx_gr_score(&chunks[i], terms, term_count);
    nx_gr_sort(chunks, chunk_count);
    for (i = 0U; i < chunk_count && selected < NX_GR_MAX_EVIDENCE; ++i) {
        if (chunks[i].score == 0U) break;
        out_answer->evidence[selected].chunk_id = chunks[i].id;
        out_answer->evidence[selected].score = chunks[i].score;
        (void)nx_gr_copy(out_answer->evidence[selected].source, sizeof(out_answer->evidence[selected].source), chunks[i].source);
        (void)nx_gr_copy(out_answer->evidence[selected].text, sizeof(out_answer->evidence[selected].text), chunks[i].text);
        score_sum += chunks[i].score;
        ++selected;
    }
    if (selected == 0U || chunks[0].score < 20U) {
        out_answer->status = NX_GR_INSUFFICIENT_EVIDENCE;
        (void)nx_gr_copy(out_answer->answer, sizeof(out_answer->answer),
                         "No encontré evidencia suficiente para responder sin especular.");
        (void)nx_gr_copy(out_answer->limitations, sizeof(out_answer->limitations),
                         "La respuesta fue rechazada por falta de coincidencia verificable.");
        return out_answer->status;
    }
    out_answer->evidence_count = selected;
    out_answer->contradiction_count = nx_gr_detect_contradictions(chunks, selected);
    (void)nx_gr_copy(out_answer->answer, sizeof(out_answer->answer), "Síntesis fundamentada: ");
    for (i = 0U; i < selected; ++i) {
        if (i > 0U) (void)nx_gr_append(out_answer->answer, sizeof(out_answer->answer), " ");
        (void)nx_gr_append(out_answer->answer, sizeof(out_answer->answer), out_answer->evidence[i].text);
    }
    (void)nx_gr_copy(out_answer->explanation, sizeof(out_answer->explanation),
                     "La respuesta combina los fragmentos con mayor coincidencia conceptual. ");
    if (selected > 1U) {
        (void)nx_gr_append(out_answer->explanation, sizeof(out_answer->explanation),
                           "Se fusionaron múltiples evidencias compatibles para evitar depender de un solo fragmento.");
    } else {
        (void)nx_gr_append(out_answer->explanation, sizeof(out_answer->explanation),
                           "Solo un fragmento alcanzó el umbral de relevancia.");
    }
    out_answer->confidence = score_sum / selected;
    if (selected > 1U && out_answer->confidence <= 90U) out_answer->confidence += 10U;
    if (out_answer->contradiction_count > 0U && out_answer->confidence >= 20U) out_answer->confidence -= 20U;
    if (out_answer->confidence > 100U) out_answer->confidence = 100U;
    (void)nx_gr_copy(out_answer->limitations, sizeof(out_answer->limitations),
                     "El razonamiento se limita a la evidencia textual persistida; no interpreta información visual ausente de la transcripción.");
    out_answer->status = NX_GR_OK;
    return out_answer->status;
}

const char* NxGroundedReasoning_StatusName(NxGroundedReasoningStatus status)
{
    switch (status) {
        case NX_GR_OK: return "OK";
        case NX_GR_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_GR_SOURCE_NOT_FOUND: return "SOURCE_NOT_FOUND";
        case NX_GR_PARSE_ERROR: return "PARSE_ERROR";
        case NX_GR_INSUFFICIENT_EVIDENCE: return "INSUFFICIENT_EVIDENCE";
        case NX_GR_IO_ERROR: return "IO_ERROR";
        default: return "UNKNOWN";
    }
}
