#include "Nexiora/Multimodal/NxMultimodalCognitiveIntegration.h"
#include "Nexiora/NCOS/NxConceptGraph.h"
#include "Nexiora/NCOS/NxConceptRegistry.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#endif

#define NX_MMCI_MAX_CONTENT 131072U
#define NX_MMCI_MAX_CHUNKS 128U
#define NX_MMCI_MAX_CHUNK 1024U
#define NX_MMCI_MAX_TERMS 256U
#define NX_MMCI_MAX_TERM 64U
#define NX_MMCI_TOP_CONCEPTS 12U

typedef struct NxMmciTerm {
    char text[NX_MMCI_MAX_TERM];
    unsigned int count;
    unsigned int first_chunk;
} NxMmciTerm;

static int nx_mmci_copy(char* dst, size_t dst_size, const char* src)
{
    size_t n;
    if (dst == NULL || dst_size == 0U || src == NULL) return 0;
    n = strlen(src);
    if (n >= dst_size) return 0;
    memcpy(dst, src, n + 1U);
    return 1;
}

static int nx_mmci_append(char* dst, size_t dst_size, const char* src)
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

static int nx_mmci_join(char* dst, size_t dst_size, const char* left, const char* right)
{
    size_t a;
    size_t b;
    int need_sep;
    if (dst == NULL || left == NULL || right == NULL || dst_size == 0U) return 0;
    a = strlen(left);
    b = strlen(right);
    need_sep = (a > 0U && left[a - 1U] != '/' && left[a - 1U] != '\\') ? 1 : 0;
    if (a + (size_t)need_sep + b >= dst_size) return 0;
    memcpy(dst, left, a);
    if (need_sep != 0) dst[a++] = '/';
    memcpy(dst + a, right, b + 1U);
    return 1;
}

static int nx_mmci_mkdir_if_needed(const char* path)
{
    if (NX_MKDIR(path) == 0) return 1;
    return errno == EEXIST ? 1 : 0;
}

static int nx_mmci_make_dirs(const char* root, const char* id, char* out, size_t out_size)
{
    char knowledge[NX_MMCI_MAX_PATH];
    char cognitive[NX_MMCI_MAX_PATH];
    if (!nx_mmci_join(knowledge, sizeof(knowledge), root, "Knowledge")) return 0;
    if (!nx_mmci_mkdir_if_needed(knowledge)) return 0;
    if (!nx_mmci_join(cognitive, sizeof(cognitive), knowledge, "MultimodalCognitive")) return 0;
    if (!nx_mmci_mkdir_if_needed(cognitive)) return 0;
    if (!nx_mmci_join(out, out_size, cognitive, id)) return 0;
    return nx_mmci_mkdir_if_needed(out);
}

static void nx_mmci_slug(const char* input, char* output, size_t output_size)
{
    size_t i;
    size_t j = 0U;
    int underscore = 0;
    if (output == NULL || output_size == 0U) return;
    output[0] = '\0';
    if (input == NULL) return;
    for (i = 0U; input[i] != '\0' && j + 1U < output_size; ++i) {
        unsigned char c = (unsigned char)input[i];
        if (isalnum(c) != 0) {
            output[j++] = (char)tolower(c);
            underscore = 0;
        } else if (j > 0U && underscore == 0) {
            output[j++] = '_';
            underscore = 1;
        }
    }
    while (j > 0U && output[j - 1U] == '_') --j;
    output[j] = '\0';
    if (j == 0U && output_size > 1U) {
        output[0] = 'x';
        output[1] = '\0';
    }
}

static int nx_mmci_read_file(const char* path, char* buffer, size_t size, size_t* used)
{
    FILE* file;
    size_t n;
    if (path == NULL || buffer == NULL || size < 2U || used == NULL) return 0;
    file = fopen(path, "rb");
    if (file == NULL) return 0;
    n = fread(buffer, 1U, size - 1U, file);
    if (ferror(file) != 0) {
        fclose(file);
        return 0;
    }
    buffer[n] = '\0';
    *used = n;
    return fclose(file) == 0 ? 1 : 0;
}

static const char* nx_mmci_content_begin(const char* text)
{
    const char* p = strstr(text, "content_begin\n");
    return p == NULL ? NULL : p + strlen("content_begin\n");
}

static size_t nx_mmci_extract_content(char* text)
{
    const char* begin = nx_mmci_content_begin(text);
    char* end;
    size_t n;
    if (begin == NULL) return 0U;
    end = strstr((char*)begin, "\ncontent_end");
    if (end == NULL) return 0U;
    n = (size_t)(end - begin);
    memmove(text, begin, n);
    text[n] = '\0';
    return n;
}

static unsigned int nx_mmci_split_chunks(const char* content,
                                         char chunks[][NX_MMCI_MAX_CHUNK],
                                         unsigned int max_chunks)
{
    unsigned int count = 0U;
    size_t start = 0U;
    size_t i;
    size_t len = strlen(content);
    for (i = 0U; i <= len && count < max_chunks; ++i) {
        int boundary = (i == len || content[i] == '.' || content[i] == '!' || content[i] == '?' || content[i] == '\n');
        if (boundary != 0) {
            size_t end = i;
            size_t n;
            while (start < end && isspace((unsigned char)content[start]) != 0) ++start;
            while (end > start && isspace((unsigned char)content[end - 1U]) != 0) --end;
            n = end - start;
            if (n >= 3U) {
                if (n >= NX_MMCI_MAX_CHUNK) n = NX_MMCI_MAX_CHUNK - 1U;
                memcpy(chunks[count], content + start, n);
                chunks[count][n] = '\0';
                ++count;
            }
            start = i + 1U;
        }
    }
    return count;
}

static int nx_mmci_stopword(const char* word)
{
    static const char* const words[] = {
        "para", "como", "este", "esta", "esto", "desde", "hasta", "entre", "sobre", "donde",
        "cuando", "porque", "pero", "tambien", "tiene", "tener", "del", "las", "los", "una",
        "uno", "unos", "unas", "que", "con", "por", "sin", "sus", "son", "sea", "ser", "the",
        "and", "for", "with", "from", "this", "that", "into", "not", "are", "was", "were"
    };
    size_t i;
    for (i = 0U; i < sizeof(words) / sizeof(words[0]); ++i) {
        if (strcmp(word, words[i]) == 0) return 1;
    }
    return 0;
}

static void nx_mmci_add_term(NxMmciTerm* terms,
                             unsigned int* term_count,
                             const char* word,
                             unsigned int chunk_index)
{
    unsigned int i;
    if (strlen(word) < 4U || nx_mmci_stopword(word) != 0) return;
    for (i = 0U; i < *term_count; ++i) {
        if (strcmp(terms[i].text, word) == 0) {
            ++terms[i].count;
            return;
        }
    }
    if (*term_count < NX_MMCI_MAX_TERMS) {
        if (nx_mmci_copy(terms[*term_count].text, sizeof(terms[*term_count].text), word) != 0) {
            terms[*term_count].count = 1U;
            terms[*term_count].first_chunk = chunk_index;
            ++(*term_count);
        }
    }
}

static void nx_mmci_collect_terms(char chunks[][NX_MMCI_MAX_CHUNK],
                                  unsigned int chunk_count,
                                  NxMmciTerm* terms,
                                  unsigned int* term_count)
{
    unsigned int c;
    *term_count = 0U;
    for (c = 0U; c < chunk_count; ++c) {
        char word[NX_MMCI_MAX_TERM];
        size_t w = 0U;
        size_t i;
        size_t len = strlen(chunks[c]);
        for (i = 0U; i <= len; ++i) {
            unsigned char ch = (unsigned char)chunks[c][i];
            if (isalnum(ch) != 0 && w + 1U < sizeof(word)) {
                word[w++] = (char)tolower(ch);
            } else if (w > 0U) {
                word[w] = '\0';
                nx_mmci_add_term(terms, term_count, word, c);
                w = 0U;
            }
        }
    }
}

static void nx_mmci_sort_terms(NxMmciTerm* terms, unsigned int count)
{
    unsigned int i;
    unsigned int j;
    for (i = 0U; i < count; ++i) {
        for (j = i + 1U; j < count; ++j) {
            if (terms[j].count > terms[i].count ||
                (terms[j].count == terms[i].count && strcmp(terms[j].text, terms[i].text) < 0)) {
                NxMmciTerm tmp = terms[i];
                terms[i] = terms[j];
                terms[j] = tmp;
            }
        }
    }
}

static int nx_mmci_write_evidence(const char* path,
                                  const char* source,
                                  char chunks[][NX_MMCI_MAX_CHUNK],
                                  unsigned int chunk_count)
{
    FILE* file = fopen(path, "wb");
    unsigned int i;
    if (file == NULL) return 0;
    for (i = 0U; i < chunk_count; ++i) {
        if (fprintf(file, "chunk=%u\nsource=%s\ntext=%s\n---\n", i + 1U, source, chunks[i]) < 0) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0 ? 1 : 0;
}

static unsigned int nx_mmci_register_concepts(const char* root,
                                              const char* domain,
                                              const char* source,
                                              char chunks[][NX_MMCI_MAX_CHUNK],
                                              NxMmciTerm* terms,
                                              unsigned int term_count,
                                              unsigned int* relations)
{
    unsigned int limit = term_count < NX_MMCI_TOP_CONCEPTS ? term_count : NX_MMCI_TOP_CONCEPTS;
    unsigned int i;
    unsigned int written = 0U;
    *relations = 0U;
    for (i = 0U; i < limit; ++i) {
        NxConceptCard card;
        char purpose[256];
        int confidence = terms[i].count >= 5U ? 90 : 60 + (int)(terms[i].count * 6U);
        if (confidence > 90) confidence = 90;
        purpose[0] = '\0';
        if (!nx_mmci_copy(purpose, sizeof(purpose), "Concepto extraido de evidencia multimodal: ")) continue;
        if (!nx_mmci_append(purpose, sizeof(purpose), source)) continue;
        if (NxConcept_Upsert(root, domain, terms[i].text,
                            chunks[terms[i].first_chunk], purpose,
                            "extraido_de evidencia_multimodal",
                            (double)confidence / 100.0, &card) != 0) {
            ++written;
        }
    }
    for (i = 1U; i < limit; ++i) {
        NxConceptGraphEdge edge;
        if (NxConceptGraph_Link(root, domain, terms[i - 1U].text, "co_occurs_with",
                                terms[i].text, source, 60, &edge) != 0) {
            ++(*relations);
        }
    }
    return written;
}

static int nx_mmci_write_report(const char* path,
                                const char* source,
                                const char* domain,
                                const NxMmciAnalysisResult* result,
                                const NxMmciTerm* terms,
                                unsigned int term_count)
{
    FILE* file = fopen(path, "wb");
    unsigned int limit = term_count < NX_MMCI_TOP_CONCEPTS ? term_count : NX_MMCI_TOP_CONCEPTS;
    unsigned int i;
    if (file == NULL) return 0;
    if (fprintf(file,
                "# Multimodal Cognitive Analysis\n\n"
                "- Source: `%s`\n- Domain: `%s`\n- Chunks: %u\n- Concepts: %u\n"
                "- Relations: %u\n- Confidence: %u%%\n\n## Extracted concepts\n",
                source, domain, result->chunks_written, result->concepts_written,
                result->relations_written, result->confidence) < 0) {
        fclose(file);
        return 0;
    }
    for (i = 0U; i < limit; ++i) {
        if (fprintf(file, "- %s (%u occurrences)\n", terms[i].text, terms[i].count) < 0) {
            fclose(file);
            return 0;
        }
    }
    return fclose(file) == 0 ? 1 : 0;
}

NxMmciStatus NxMmci_Analyze(const char* root,
                            const char* knowledge_path,
                            const char* domain,
                            NxMmciAnalysisResult* result)
{
    char* content;
    size_t used = 0U;
    char id[128];
    char chunks[NX_MMCI_MAX_CHUNKS][NX_MMCI_MAX_CHUNK];
    NxMmciTerm terms[NX_MMCI_MAX_TERMS];
    unsigned int chunk_count;
    unsigned int term_count;
    unsigned int relations;
    if (root == NULL || knowledge_path == NULL || domain == NULL || result == NULL ||
        root[0] == '\0' || knowledge_path[0] == '\0' || domain[0] == '\0') return NX_MMCI_INVALID_ARGUMENT;
    memset(result, 0, sizeof(*result));
    content = (char*)malloc(NX_MMCI_MAX_CONTENT);
    if (content == NULL) {
        result->status = NX_MMCI_IO_ERROR;
        return result->status;
    }
    if (!nx_mmci_read_file(knowledge_path, content, NX_MMCI_MAX_CONTENT, &used)) {
        free(content);
        result->status = NX_MMCI_NOT_FOUND;
        nx_mmci_copy(result->message, sizeof(result->message), "Knowledge file not found or unreadable.");
        return result->status;
    }
    if (used == 0U || nx_mmci_extract_content(content) == 0U) {
        free(content);
        result->status = NX_MMCI_FORMAT_ERROR;
        nx_mmci_copy(result->message, sizeof(result->message), "Expected nxknowledge/1 content markers.");
        return result->status;
    }
    nx_mmci_slug(knowledge_path, id, sizeof(id));
    if (!nx_mmci_make_dirs(root, id, result->analysis_dir, sizeof(result->analysis_dir)) ||
        !nx_mmci_join(result->evidence_path, sizeof(result->evidence_path), result->analysis_dir, "evidence.nxevidence") ||
        !nx_mmci_join(result->report_path, sizeof(result->report_path), result->analysis_dir, "report.md")) {
        free(content);
        result->status = NX_MMCI_OUTPUT_TOO_SMALL;
        return result->status;
    }
    chunk_count = nx_mmci_split_chunks(content, chunks, NX_MMCI_MAX_CHUNKS);
    if (chunk_count == 0U) {
        free(content);
        result->status = NX_MMCI_NO_EVIDENCE;
        nx_mmci_copy(result->message, sizeof(result->message), "No usable textual evidence found.");
        return result->status;
    }
    nx_mmci_collect_terms(chunks, chunk_count, terms, &term_count);
    nx_mmci_sort_terms(terms, term_count);
    if (!nx_mmci_write_evidence(result->evidence_path, knowledge_path, chunks, chunk_count)) {
        free(content);
        result->status = NX_MMCI_IO_ERROR;
        return result->status;
    }
    result->chunks_written = chunk_count;
    result->concepts_written = nx_mmci_register_concepts(root, domain, knowledge_path, chunks, terms, term_count, &relations);
    result->relations_written = relations;
    result->confidence = term_count == 0U ? 45U : (result->concepts_written > 4U ? 82U : 65U);
    if (!nx_mmci_write_report(result->report_path, knowledge_path, domain, result, terms, term_count)) {
        free(content);
        result->status = NX_MMCI_IO_ERROR;
        return result->status;
    }
    free(content);
    result->status = NX_MMCI_OK;
    nx_mmci_copy(result->message, sizeof(result->message), "Evidence segmented and integrated into concept registry and graph.");
    return result->status;
}

static unsigned int nx_mmci_question_terms(const char* question, char terms[][NX_MMCI_MAX_TERM], unsigned int max_terms)
{
    unsigned int count = 0U;
    char word[NX_MMCI_MAX_TERM];
    size_t w = 0U;
    size_t i;
    size_t len = strlen(question);
    for (i = 0U; i <= len && count < max_terms; ++i) {
        unsigned char c = (unsigned char)question[i];
        if (isalnum(c) != 0 && w + 1U < sizeof(word)) word[w++] = (char)tolower(c);
        else if (w > 0U) {
            word[w] = '\0';
            if (strlen(word) >= 4U && nx_mmci_stopword(word) == 0 && nx_mmci_copy(terms[count], NX_MMCI_MAX_TERM, word) != 0) ++count;
            w = 0U;
        }
    }
    return count;
}

static unsigned int nx_mmci_score(const char* text, char terms[][NX_MMCI_MAX_TERM], unsigned int term_count)
{
    char lower[NX_MMCI_MAX_CHUNK];
    size_t i;
    unsigned int score = 0U;
    size_t n = strlen(text);
    if (n >= sizeof(lower)) n = sizeof(lower) - 1U;
    for (i = 0U; i < n; ++i) lower[i] = (char)tolower((unsigned char)text[i]);
    lower[n] = '\0';
    for (i = 0U; i < term_count; ++i) if (strstr(lower, terms[i]) != NULL) ++score;
    return score;
}

NxMmciStatus NxMmci_Query(const char* root,
                          const char* analysis_id,
                          const char* question,
                          NxMmciQueryResult* result)
{
    char dir[NX_MMCI_MAX_PATH];
    char path[NX_MMCI_MAX_PATH];
    char base[NX_MMCI_MAX_PATH];
    FILE* file;
    char line[NX_MMCI_MAX_CHUNK + 32U];
    char current[NX_MMCI_MAX_CHUNK];
    char best[NX_MMCI_MAX_CHUNK];
    unsigned int current_id = 0U;
    unsigned int best_id = 0U;
    unsigned int best_score = 0U;
    unsigned int matches = 0U;
    char terms[24][NX_MMCI_MAX_TERM];
    unsigned int term_count;
    if (root == NULL || analysis_id == NULL || question == NULL || result == NULL ||
        root[0] == '\0' || analysis_id[0] == '\0' || question[0] == '\0') return NX_MMCI_INVALID_ARGUMENT;
    memset(result, 0, sizeof(*result));
    if (!nx_mmci_join(base, sizeof(base), root, "Knowledge/MultimodalCognitive") ||
        !nx_mmci_join(dir, sizeof(dir), base, analysis_id) ||
        !nx_mmci_join(path, sizeof(path), dir, "evidence.nxevidence")) {
        result->status = NX_MMCI_OUTPUT_TOO_SMALL;
        return result->status;
    }
    file = fopen(path, "rb");
    if (file == NULL) {
        result->status = NX_MMCI_NOT_FOUND;
        nx_mmci_copy(result->message, sizeof(result->message), "Analysis evidence not found.");
        return result->status;
    }
    term_count = nx_mmci_question_terms(question, terms, 24U);
    current[0] = '\0';
    best[0] = '\0';
    while (fgets(line, (int)sizeof(line), file) != NULL) {
        if (sscanf(line, "chunk=%u", &current_id) == 1) continue;
        if (strncmp(line, "text=", 5U) == 0) {
            unsigned int score;
            size_t n = strlen(line + 5U);
            while (n > 0U && (line[5U + n - 1U] == '\n' || line[5U + n - 1U] == '\r')) --n;
            if (n >= sizeof(current)) n = sizeof(current) - 1U;
            memcpy(current, line + 5U, n);
            current[n] = '\0';
            score = nx_mmci_score(current, terms, term_count);
            if (score > 0U) ++matches;
            if (score > best_score) {
                best_score = score;
                best_id = current_id;
                nx_mmci_copy(best, sizeof(best), current);
            }
        }
    }
    fclose(file);
    if (best_score == 0U) {
        result->status = NX_MMCI_NO_EVIDENCE;
        nx_mmci_copy(result->message, sizeof(result->message), "No supporting evidence matched the question.");
        return result->status;
    }
    result->answer[0] = '\0';
    nx_mmci_append(result->answer, sizeof(result->answer), best);
    nx_mmci_append(result->answer, sizeof(result->answer), "\n\nEvidence: chunk ");
    {
        char number[32];
        if (snprintf(number, sizeof(number), "%u", best_id) < 0 || !nx_mmci_append(result->answer, sizeof(result->answer), number)) {
            result->status = NX_MMCI_OUTPUT_TOO_SMALL;
            return result->status;
        }
    }
    nx_mmci_append(result->answer, sizeof(result->answer), " from ");
    nx_mmci_append(result->answer, sizeof(result->answer), path);
    result->matched_chunks = matches;
    result->confidence = 55U + (best_score * 10U > 40U ? 40U : best_score * 10U);
    nx_mmci_copy(result->evidence_path, sizeof(result->evidence_path), path);
    nx_mmci_copy(result->message, sizeof(result->message), "Answer grounded in persisted multimodal evidence.");
    result->status = NX_MMCI_OK;
    return result->status;
}

const char* NxMmci_StatusToString(NxMmciStatus status)
{
    switch (status) {
        case NX_MMCI_OK: return "OK";
        case NX_MMCI_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
        case NX_MMCI_NOT_FOUND: return "NOT_FOUND";
        case NX_MMCI_FORMAT_ERROR: return "FORMAT_ERROR";
        case NX_MMCI_IO_ERROR: return "IO_ERROR";
        case NX_MMCI_NO_EVIDENCE: return "NO_EVIDENCE";
        case NX_MMCI_OUTPUT_TOO_SMALL: return "OUTPUT_TOO_SMALL";
        default: return "UNKNOWN";
    }
}
