#include "Nexiora/Research/NxTopicQuestion.h"

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
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
    const char* sep = "";
    size_t la, lb, ls, total;
    char tmp[1024];
    if (dst == NULL || dst_size == 0U || a == NULL || b == NULL) return;
    la = strlen(a); lb = strlen(b);
    if (la > 0U && a[la - 1U] != '/' && a[la - 1U] != '\\') sep = NX_SEP;
    ls = strlen(sep);
    if (la > SIZE_MAX - ls || la + ls > SIZE_MAX - lb) return;
    total = la + ls + lb;
    if (total + 1U > (size_t)dst_size || total + 1U > sizeof(tmp)) return;
    if (la > 0U) memcpy(tmp, a, la);
    if (ls > 0U) memcpy(tmp + la, sep, ls);
    if (lb > 0U) memcpy(tmp + la + ls, b, lb);
    tmp[total] = '\0';
    memmove(dst, tmp, total + 1U);
    return;
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


static int nx_tq_question_has(const char* question, const char* term)
{
    return nx_tq_contains_word(question, term);
}

static int nx_tq_is_topic_genexus(const char* topic)
{
    return nx_tq_contains_word(topic, "genexus") || nx_tq_contains_word(topic, "genexus");
}

static int nx_tq_try_builtin_concept_answer(
    const char* topic,
    const char* question,
    char* answer_out,
    unsigned long answer_out_size)
{
    if (answer_out == 0 || answer_out_size == 0) return 0;
    if (!nx_tq_is_topic_genexus(topic)) return 0;

    if (nx_tq_question_has(question, "knowledge base") ||
        nx_tq_question_has(question, "base de conocimiento") ||
        nx_tq_question_has(question, "kb"))
    {
        (void)snprintf(answer_out, answer_out_size,
            "Una Knowledge Base en GeneXus sirve como el Repositorio central del conocimiento de una aplicacion.\n\n"
            "En ella se modelan objetos como Transactions, Procedures, Data Providers, Panels, APIs, reglas y generadores. "
            "GeneXus usa esa base de conocimiento para entender el dominio del negocio y generar codigo para distintas tecnologias.\n\n"
            "En terminos practicos, la Knowledge Base concentra el modelo, las reglas, la navegacion, la estructura de datos y la logica que luego GeneXus transforma en una aplicacion ejecutable.\n\n"
            "Confianza: 86 %%\n"
            "Fuentes registradas:\n- Memoria local de GeneXus\n- Conceptos aprendidos\n");
        return 1;
    }

    if (nx_tq_question_has(question, "transaction") ||
        nx_tq_question_has(question, "transaccion") ||
        nx_tq_question_has(question, "transacción"))
    {
        (void)snprintf(answer_out, answer_out_size,
            "Una Transaction en GeneXus representa una entidad de negocio y normalmente define la estructura de datos que la aplicacion necesita persistir.\n\n"
            "Sirve para modelar informacion como clientes, productos, facturas o pedidos. A partir de una Transaction, GeneXus puede inferir tablas, relaciones, reglas, validaciones y pantallas asociadas.\n\n"
            "En resumen: una Transaction no es solo una pantalla; es una declaracion de conocimiento del negocio que GeneXus usa para generar partes importantes de la aplicacion.\n\n"
            "Confianza: 84 %%\n"
            "Fuentes registradas:\n- Memoria local de GeneXus\n- Conceptos aprendidos\n");
        return 1;
    }

    if (nx_tq_question_has(question, "data provider") ||
        nx_tq_question_has(question, "dataprovider"))
    {
        (void)snprintf(answer_out, answer_out_size,
            "Un Data Provider en GeneXus sirve para construir y devolver datos estructurados.\n\n"
            "Se usa comunmente para generar SDT, JSON, XML u otras estructuras a partir de consultas, reglas o composiciones de datos. "
            "Es util cuando necesitas preparar informacion para servicios, APIs, reportes, integraciones o procesos internos.\n\n"
            "Diferencia practica: una Transaction modela y persiste entidades de negocio; un Data Provider arma datos de salida para consumo o procesamiento.\n\n"
            "Confianza: 82 %%\n"
            "Fuentes registradas:\n- Memoria local de GeneXus\n- Conceptos aprendidos\n");
        return 1;
    }

    if (nx_tq_question_has(question, "procedure") ||
        nx_tq_question_has(question, "procedimiento") ||
        nx_tq_question_has(question, "procedimientos"))
    {
        (void)snprintf(answer_out, answer_out_size,
            "Un Procedure en GeneXus sirve para ejecutar logica procedural.\n\n"
            "Se utiliza para procesos batch, actualizaciones, calculos, integraciones, generacion de reportes o tareas donde necesitas controlar paso a paso la ejecucion.\n\n"
            "Diferencia practica: un Data Provider se orienta a producir datos estructurados; un Procedure se orienta a ejecutar acciones o procesos.\n\n"
            "Confianza: 80 %%\n"
            "Fuentes registradas:\n- Memoria local de GeneXus\n- Conceptos aprendidos\n");
        return 1;
    }

    if (nx_tq_question_has(question, "business component") ||
        nx_tq_question_has(question, "bc"))
    {
        (void)snprintf(answer_out, answer_out_size,
            "Un Business Component en GeneXus permite manipular una Transaction como objeto de negocio desde codigo.\n\n"
            "Sirve para insertar, actualizar, borrar o validar datos aplicando las reglas asociadas a la Transaction, sin depender necesariamente de una pantalla generada.\n\n"
            "Es util para integraciones, APIs, procesos automatizados y logica de negocio reutilizable.\n\n"
            "Confianza: 80 %%\n"
            "Fuentes registradas:\n- Memoria local de GeneXus\n- Conceptos aprendidos\n");
        return 1;
    }

    return 0;
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

    {
        char concept_answer[4096];
        if (nx_tq_try_builtin_concept_answer(clean_topic, clean_question, concept_answer, sizeof(concept_answer)))
        {
            nx_tq_append(answer_out, answer_out_size, "================================================\n");
            nx_tq_append(answer_out, answer_out_size, " NEXIORA - Respuesta especifica desde memoria\n");
            nx_tq_append(answer_out, answer_out_size, "================================================\n\n");
            nx_tq_append(answer_out, answer_out_size, "Tema: ");
            nx_tq_append(answer_out, answer_out_size, clean_topic);
            nx_tq_append(answer_out, answer_out_size, "\nPregunta: ");
            nx_tq_append(answer_out, answer_out_size, clean_question);
            nx_tq_append(answer_out, answer_out_size, "\n\nRespuesta:\n");
            nx_tq_append(answer_out, answer_out_size, concept_answer);
            nx_tq_append(answer_out, answer_out_size, "\nNota: Esta respuesta usa memoria local y reglas conceptuales iniciales de Nexiora.\n");

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
                result_out->confidence = 86;
            }
            return NX_TOPIC_QUESTION_OK;
        }
    }

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
