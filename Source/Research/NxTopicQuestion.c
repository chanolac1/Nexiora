#include "Nexiora/Research/NxTopicQuestion.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define NX_SEP "\\"
#else
#define NX_SEP "/"
#endif

#define NX_TQ_MAX_FACTS 8
#define NX_TQ_MAX_CONCEPTS 12
#define NX_TQ_MAX_SOURCES 6

typedef struct NxTqEntry
{
    char text[1024];
    int score;
} NxTqEntry;

static void nx_tq_trim_copy(char* dst, unsigned long dst_size, const char* src)
{
    const char* start;
    const char* end;
    unsigned long len;
    if (dst == 0 || dst_size == 0) return;
    dst[0] = '\0';
    if (src == 0) return;
    start = src;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') start++;
    end = start + strlen(start);
    while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\r' || *(end - 1) == '\n')) end--;
    len = (unsigned long)(end - start);
    if (len >= dst_size) len = dst_size - 1;
    memcpy(dst, start, len);
    dst[len] = '\0';
}

static void nx_tq_slugify(char* dst, unsigned long dst_size, const char* text)
{
    unsigned long i;
    unsigned long out = 0;
    if (dst == 0 || dst_size == 0) return;
    dst[0] = '\0';
    if (text == 0) return;
    for (i = 0; text[i] != '\0' && out + 1 < dst_size; ++i)
    {
        unsigned char ch = (unsigned char)text[i];
        if (isalnum(ch)) dst[out++] = (char)tolower(ch);
        else if (out > 0 && dst[out - 1] != '-') dst[out++] = '-';
    }
    while (out > 0 && dst[out - 1] == '-') out--;
    dst[out] = '\0';
    if (dst[0] == '\0') (void)snprintf(dst, dst_size, "topic");
}

static void nx_tq_join(char* dst, unsigned long dst_size, const char* a, const char* b)
{
    size_t len;
    const char* sep;
    if (dst == 0 || dst_size == 0) return;
    if (a == 0) a = "";
    if (b == 0) b = "";
    if (a[0] == '\0') { (void)snprintf(dst, dst_size, "%s", b); return; }
    len = strlen(a);
    sep = (len > 0 && (a[len - 1] == '\\' || a[len - 1] == '/')) ? "" : NX_SEP;
    (void)snprintf(dst, dst_size, "%s%s%s", a, sep, b);
}

static int nx_tq_file_exists(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (f == 0) return 0;
    fclose(f);
    return 1;
}

static void nx_tq_lower_ascii(char* dst, unsigned long dst_size, const char* src)
{
    unsigned long i;
    if (dst == 0 || dst_size == 0) return;
    dst[0] = '\0';
    if (src == 0) return;
    for (i = 0; src[i] != '\0' && i + 1 < dst_size; ++i)
    {
        dst[i] = (char)tolower((unsigned char)src[i]);
    }
    dst[i] = '\0';
}

static int nx_tq_contains_word(const char* haystack, const char* needle)
{
    char h[2048];
    char n[128];
    if (haystack == 0 || needle == 0 || needle[0] == '\0') return 0;
    nx_tq_lower_ascii(h, sizeof(h), haystack);
    nx_tq_lower_ascii(n, sizeof(n), needle);
    return strstr(h, n) != 0;
}

static int nx_tq_is_stopword(const char* word)
{
    static const char* stop[] = {
        "que", "qué", "sabes", "sobre", "del", "de", "la", "el", "los", "las",
        "una", "uno", "un", "como", "cómo", "para", "por", "con", "sin", "cual",
        "cuál", "dame", "explica", "explicame", "explícame", "es", "son", "sus",
        "tiene", "puede", "puedo", "usar", "uso", "en", "y", "o", "a", "me", "lo",
        0
    };
    int i;
    char lower[128];
    nx_tq_lower_ascii(lower, sizeof(lower), word);
    for (i = 0; stop[i] != 0; ++i)
    {
        if (strcmp(lower, stop[i]) == 0) return 1;
    }
    return 0;
}

static int nx_tq_extract_keywords(const char* question, char keywords[][64], int max_keywords)
{
    int count = 0;
    char word[64];
    int w = 0;
    unsigned long i;
    if (question == 0 || keywords == 0) return 0;
    for (i = 0;; ++i)
    {
        unsigned char ch = (unsigned char)question[i];
        if (isalnum(ch) || ch == '_' || ch == '-')
        {
            if (w + 1 < (int)sizeof(word)) word[w++] = (char)ch;
        }
        else
        {
            if (w > 0)
            {
                int duplicate = 0;
                int k;
                word[w] = '\0';
                if (w >= 3 && !nx_tq_is_stopword(word))
                {
                    for (k = 0; k < count; ++k)
                    {
                        if (strcmp(keywords[k], word) == 0) duplicate = 1;
                    }
                    if (!duplicate && count < max_keywords)
                    {
                        (void)snprintf(keywords[count], 64, "%s", word);
                        count++;
                    }
                }
                w = 0;
            }
            if (ch == '\0') break;
        }
    }
    return count;
}

static int nx_tq_json_value(const char* line, const char* key, char* dst, unsigned long dst_size)
{
    char pattern[64];
    const char* p;
    const char* start;
    unsigned long out = 0;
    if (line == 0 || key == 0 || dst == 0 || dst_size == 0) return 0;
    dst[0] = '\0';
    (void)snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
    p = strstr(line, pattern);
    if (p == 0) return 0;
    start = p + strlen(pattern);
    while (*start != '\0' && out + 1 < dst_size)
    {
        if (*start == '"') break;
        if (*start == '\\' && *(start + 1) != '\0') start++;
        dst[out++] = *start++;
    }
    dst[out] = '\0';
    return out > 0;
}

static void nx_tq_add_entry(NxTqEntry* entries, int* count, int max_count, const char* text, int score)
{
    int i;
    if (entries == 0 || count == 0 || text == 0 || text[0] == '\0') return;
    for (i = 0; i < *count; ++i)
    {
        if (strcmp(entries[i].text, text) == 0)
        {
            if (score > entries[i].score) entries[i].score = score;
            return;
        }
    }
    if (*count >= max_count) return;
    (void)snprintf(entries[*count].text, sizeof(entries[*count].text), "%s", text);
    entries[*count].score = score;
    (*count)++;
}

static void nx_tq_sort_entries(NxTqEntry* entries, int count)
{
    int i;
    int j;
    for (i = 0; i < count; ++i)
    {
        for (j = i + 1; j < count; ++j)
        {
            if (entries[j].score > entries[i].score)
            {
                NxTqEntry tmp = entries[i];
                entries[i] = entries[j];
                entries[j] = tmp;
            }
        }
    }
}

static int nx_tq_score_text(const char* text, char keywords[][64], int keyword_count)
{
    int score = 0;
    int i;
    if (text == 0) return 0;
    for (i = 0; i < keyword_count; ++i)
    {
        if (nx_tq_contains_word(text, keywords[i])) score += 10;
    }
    if (score == 0) score = 1;
    return score;
}

static void nx_tq_append(char* dst, unsigned long dst_size, const char* text)
{
    size_t used;
    if (dst == 0 || dst_size == 0 || text == 0) return;
    used = strlen(dst);
    if (used + 1 >= dst_size) return;
    (void)snprintf(dst + used, dst_size - used, "%s", text);
}

NxTopicQuestionStatus NxTopicQuestion_Ask(
    const char* topic,
    const char* question,
    char* answer_out,
    unsigned long answer_out_size,
    NxTopicQuestionResult* result_out)
{
    char clean_topic[128];
    char clean_question[512];
    char slug[128];
    char knowledge_dir[512];
    char topics_dir[512];
    char topic_dir[512];
    char memory_path[512];
    char answer_path[512];
    FILE* memory;
    FILE* answer_file;
    char line[2048];
    char keywords[16][64];
    int keyword_count;
    NxTqEntry facts[NX_TQ_MAX_FACTS];
    NxTqEntry concepts[NX_TQ_MAX_CONCEPTS];
    NxTqEntry sources[NX_TQ_MAX_SOURCES];
    int fact_count = 0;
    int concept_count = 0;
    int source_count = 0;
    int i;

    if (topic == 0 || question == 0 || answer_out == 0 || answer_out_size == 0)
    {
        return NX_TOPIC_QUESTION_INVALID_ARGUMENT;
    }

    answer_out[0] = '\0';
    if (result_out != 0) memset(result_out, 0, sizeof(*result_out));
    memset(facts, 0, sizeof(facts));
    memset(concepts, 0, sizeof(concepts));
    memset(sources, 0, sizeof(sources));

    nx_tq_trim_copy(clean_topic, sizeof(clean_topic), topic);
    nx_tq_trim_copy(clean_question, sizeof(clean_question), question);
    if (clean_topic[0] == '\0' || clean_question[0] == '\0') return NX_TOPIC_QUESTION_INVALID_ARGUMENT;
    nx_tq_slugify(slug, sizeof(slug), clean_topic);

    nx_tq_join(knowledge_dir, sizeof(knowledge_dir), ".", "Knowledge");
    nx_tq_join(topics_dir, sizeof(topics_dir), knowledge_dir, "Topics");
    nx_tq_join(topic_dir, sizeof(topic_dir), topics_dir, slug);
    nx_tq_join(memory_path, sizeof(memory_path), topic_dir, "memory.jsonl");
    nx_tq_join(answer_path, sizeof(answer_path), topic_dir, "last_question_answer.txt");

    if (!nx_tq_file_exists(memory_path))
    {
        (void)snprintf(answer_out, answer_out_size,
            "No tengo conocimiento suficiente sobre '%s'. Ejecuta primero: nexiora aprende %s\n",
            clean_topic,
            clean_topic);
        return NX_TOPIC_QUESTION_NOT_FOUND;
    }

    keyword_count = nx_tq_extract_keywords(clean_question, keywords, 16);

    memory = fopen(memory_path, "rb");
    if (memory == 0) return NX_TOPIC_QUESTION_IO_FAILED;

    while (fgets(line, sizeof(line), memory) != 0)
    {
        char value[1024];
        int score;
        if (strstr(line, "\"type\":\"fact\"") != 0 && nx_tq_json_value(line, "text", value, sizeof(value)))
        {
            score = nx_tq_score_text(value, keywords, keyword_count);
            nx_tq_add_entry(facts, &fact_count, NX_TQ_MAX_FACTS, value, score);
        }
        else if (strstr(line, "\"type\":\"concept\"") != 0 && nx_tq_json_value(line, "name", value, sizeof(value)))
        {
            score = nx_tq_score_text(value, keywords, keyword_count);
            nx_tq_add_entry(concepts, &concept_count, NX_TQ_MAX_CONCEPTS, value, score);
        }
        else if (strstr(line, "\"type\":\"source\"") != 0)
        {
            char title[512];
            char url[512];
            title[0] = '\0';
            url[0] = '\0';
            (void)nx_tq_json_value(line, "title", title, sizeof(title));
            (void)nx_tq_json_value(line, "url", url, sizeof(url));
            if (title[0] != '\0')
            {
                if (url[0] != '\0')
                {
                    char combined[1024];
                    (void)snprintf(combined, sizeof(combined), "%s - %s", title, url);
                    nx_tq_add_entry(sources, &source_count, NX_TQ_MAX_SOURCES, combined, 1);
                }
                else
                {
                    nx_tq_add_entry(sources, &source_count, NX_TQ_MAX_SOURCES, title, 1);
                }
            }
        }
    }
    fclose(memory);

    nx_tq_sort_entries(facts, fact_count);
    nx_tq_sort_entries(concepts, concept_count);

    nx_tq_append(answer_out, answer_out_size, "================================================\n");
    nx_tq_append(answer_out, answer_out_size, " NEXIORA - Respuesta desde memoria\n");
    nx_tq_append(answer_out, answer_out_size, "================================================\n\n");
    nx_tq_append(answer_out, answer_out_size, "Tema: ");
    nx_tq_append(answer_out, answer_out_size, clean_topic);
    nx_tq_append(answer_out, answer_out_size, "\nPregunta: ");
    nx_tq_append(answer_out, answer_out_size, clean_question);
    nx_tq_append(answer_out, answer_out_size, "\n\nRespuesta:\n");

    if (fact_count == 0 && concept_count == 0)
    {
        nx_tq_append(answer_out, answer_out_size, "No encontre hechos suficientes en la memoria del tema.\n");
    }
    else
    {
        int used_facts = fact_count < 3 ? fact_count : 3;
        for (i = 0; i < used_facts; ++i)
        {
            nx_tq_append(answer_out, answer_out_size, "- ");
            nx_tq_append(answer_out, answer_out_size, facts[i].text);
            nx_tq_append(answer_out, answer_out_size, "\n");
        }
        if (used_facts == 0 && concept_count > 0)
        {
            nx_tq_append(answer_out, answer_out_size, "La memoria contiene conceptos relacionados, pero no hechos descriptivos suficientes.\n");
        }
    }

    nx_tq_append(answer_out, answer_out_size, "\nConceptos relacionados:\n");
    if (concept_count == 0)
    {
        nx_tq_append(answer_out, answer_out_size, "- Sin conceptos registrados.\n");
    }
    else
    {
        int used_concepts = concept_count < 8 ? concept_count : 8;
        for (i = 0; i < used_concepts; ++i)
        {
            nx_tq_append(answer_out, answer_out_size, "- ");
            nx_tq_append(answer_out, answer_out_size, concepts[i].text);
            nx_tq_append(answer_out, answer_out_size, "\n");
        }
    }

    nx_tq_append(answer_out, answer_out_size, "\nFuentes registradas:\n");
    if (source_count == 0)
    {
        nx_tq_append(answer_out, answer_out_size, "- Sin fuentes registradas.\n");
    }
    else
    {
        int used_sources = source_count < 4 ? source_count : 4;
        for (i = 0; i < used_sources; ++i)
        {
            nx_tq_append(answer_out, answer_out_size, "- ");
            nx_tq_append(answer_out, answer_out_size, sources[i].text);
            nx_tq_append(answer_out, answer_out_size, "\n");
        }
    }

    nx_tq_append(answer_out, answer_out_size, "\nConfianza: ");
    if (fact_count >= 2 && source_count >= 1) nx_tq_append(answer_out, answer_out_size, "78 %\n");
    else if (fact_count >= 1) nx_tq_append(answer_out, answer_out_size, "62 %\n");
    else nx_tq_append(answer_out, answer_out_size, "40 %\n");

    nx_tq_append(answer_out, answer_out_size, "\nNota: Respondo solo con memoria local aprendida. Si falta detalle, ejecuta una nueva investigacion del tema.\n");

    answer_file = fopen(answer_path, "wb");
    if (answer_file != 0)
    {
        fputs(answer_out, answer_file);
        fclose(answer_file);
    }

    if (result_out != 0)
    {
        (void)snprintf(result_out->topic, sizeof(result_out->topic), "%s", clean_topic);
        (void)snprintf(result_out->topic_slug, sizeof(result_out->topic_slug), "%s", slug);
        (void)snprintf(result_out->question, sizeof(result_out->question), "%s", clean_question);
        (void)snprintf(result_out->answer_path, sizeof(result_out->answer_path), "%s", answer_path);
        result_out->evidence_items = fact_count;
        result_out->concepts_used = concept_count;
        result_out->sources_used = source_count;
        result_out->confidence = (fact_count >= 2 && source_count >= 1) ? 78 : (fact_count >= 1 ? 62 : 40);
    }

    return NX_TOPIC_QUESTION_OK;
}

const char* NxTopicQuestion_StatusToString(NxTopicQuestionStatus status)
{
    switch (status)
    {
    case NX_TOPIC_QUESTION_OK: return "ok";
    case NX_TOPIC_QUESTION_INVALID_ARGUMENT: return "invalid argument";
    case NX_TOPIC_QUESTION_NOT_FOUND: return "not found";
    case NX_TOPIC_QUESTION_IO_FAILED: return "io failed";
    default: return "unknown";
    }
}
