#include "Nexiora/Cognitive/NxCognitiveCore.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <strings.h>
#endif

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#define NX_PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <sys/types.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#define NX_PATH_SEP "/"
#endif

#define NX_COG_MAX_TEXT (1024 * 512)
#define NX_COG_MAX_CHUNKS 64
#define NX_COG_MAX_CONCEPTS 80

typedef struct NxConceptCount
{
    char word[64];
    int count;
} NxConceptCount;

typedef struct NxChunkMatch
{
    char text[768];
    int score;
} NxChunkMatch;

static int nx_is_sep(char c)
{
    return c == '/' || c == '\\';
}

static void nx_copy_string(char* dst, size_t dst_size, const char* src)
{
    if (dst == 0 || dst_size == 0)
    {
        return;
    }

    if (src == 0)
    {
        dst[0] = '\0';
        return;
    }

    (void)snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static void nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    size_t a_len;
    const char* sep;

    if (dst == 0 || dst_size == 0)
    {
        return;
    }

    if (a == 0 || a[0] == '\0')
    {
        nx_copy_string(dst, dst_size, b == 0 ? "" : b);
        return;
    }

    if (b == 0 || b[0] == '\0')
    {
        nx_copy_string(dst, dst_size, a);
        return;
    }

    a_len = strlen(a);
    sep = nx_is_sep(a[a_len - 1]) ? "" : NX_PATH_SEP;
    (void)snprintf(dst, dst_size, "%s%s%s", a, sep, b);
    dst[dst_size - 1] = '\0';
}

static void nx_make_dir_if_needed(const char* path)
{
    if (path != 0 && path[0] != '\0')
    {
        (void)NX_MKDIR(path);
    }
}

static void nx_make_topic_dirs(const char* root, const char* normalized_topic, char* topic_dir, size_t topic_dir_size)
{
    char knowledge[512];
    char cognitive[512];
    char topics[512];

    nx_join(knowledge, sizeof(knowledge), root, "Knowledge");
    nx_join(cognitive, sizeof(cognitive), knowledge, "Cognitive");
    nx_join(topics, sizeof(topics), cognitive, "Topics");
    nx_join(topic_dir, topic_dir_size, topics, normalized_topic);

    nx_make_dir_if_needed(knowledge);
    nx_make_dir_if_needed(cognitive);
    nx_make_dir_if_needed(topics);
    nx_make_dir_if_needed(topic_dir);
}

static int nx_has_extension(const char* path, const char* ext)
{
    size_t lp;
    size_t le;
    if (path == 0 || ext == 0)
    {
        return 0;
    }
    lp = strlen(path);
    le = strlen(ext);
    if (lp < le)
    {
        return 0;
    }
#if defined(_WIN32)
    return _stricmp(path + lp - le, ext) == 0;
#else
    return strcasecmp(path + lp - le, ext) == 0;
#endif
}

static int nx_is_textual(const char* path)
{
    return nx_has_extension(path, ".txt") || nx_has_extension(path, ".md") ||
           nx_has_extension(path, ".markdown") || nx_has_extension(path, ".srt") ||
           nx_has_extension(path, ".vtt") || nx_has_extension(path, ".csv") ||
           nx_has_extension(path, ".json") || nx_has_extension(path, ".xml") ||
           nx_has_extension(path, ".html") || nx_has_extension(path, ".htm") ||
           nx_has_extension(path, ".c") || nx_has_extension(path, ".h") ||
           nx_has_extension(path, ".cpp") || nx_has_extension(path, ".hpp") ||
           nx_has_extension(path, ".cs") || nx_has_extension(path, ".java") ||
           nx_has_extension(path, ".js") || nx_has_extension(path, ".py") ||
           nx_has_extension(path, ".log");
}

static int nx_is_media(const char* path)
{
    return nx_has_extension(path, ".png") || nx_has_extension(path, ".jpg") ||
           nx_has_extension(path, ".jpeg") || nx_has_extension(path, ".webp") ||
           nx_has_extension(path, ".bmp") || nx_has_extension(path, ".gif") ||
           nx_has_extension(path, ".mp4") || nx_has_extension(path, ".mov") ||
           nx_has_extension(path, ".mkv") || nx_has_extension(path, ".avi") ||
           nx_has_extension(path, ".mp3") || nx_has_extension(path, ".wav") ||
           nx_has_extension(path, ".m4a");
}

static size_t nx_read_file(const char* path, char* buffer, size_t buffer_size)
{
    FILE* f;
    size_t n;

    if (path == 0 || buffer == 0 || buffer_size == 0)
    {
        return 0;
    }

    f = fopen(path, "rb");
    if (f == 0)
    {
        return 0;
    }

    n = fread(buffer, 1, buffer_size - 1, f);
    buffer[n] = '\0';
    fclose(f);
    return n;
}

static int nx_find_sidecar(const char* path, char* sidecar, size_t sidecar_size)
{
    FILE* f;
    const char* extensions[] = { ".txt", ".srt", ".vtt", 0 };
    int i;

    if (path == 0 || sidecar == 0 || sidecar_size == 0)
    {
        return 0;
    }

    for (i = 0; extensions[i] != 0; ++i)
    {
        (void)snprintf(sidecar, sidecar_size, "%s%s", path, extensions[i]);
        sidecar[sidecar_size - 1] = '\0';
        f = fopen(sidecar, "rb");
        if (f != 0)
        {
            fclose(f);
            return 1;
        }
    }

    nx_copy_string(sidecar, sidecar_size, path);
    {
        char* dot = strrchr(sidecar, '.');
        if (dot != 0)
        {
            nx_copy_string(dot, sidecar_size - (size_t)(dot - sidecar), ".txt");
            f = fopen(sidecar, "rb");
            if (f != 0)
            {
                fclose(f);
                return 1;
            }
        }
    }

    sidecar[0] = '\0';
    return 0;
}

static int nx_is_stopword(const char* word)
{
    static const char* stopwords[] = {
        "para", "pero", "como", "cuando", "donde", "este", "esta", "estos", "estas",
        "sobre", "entre", "desde", "hasta", "porque", "tambien", "tiene", "puede",
        "sirve", "hacer", "hace", "todo", "toda", "todos", "todas", "with", "that",
        "this", "from", "into", "their", "there", "what", "when", "where", "using",
        "used", "uses", "the", "and", "for", "una", "uno", "unos", "unas", "los",
        "las", "del", "que", "por", "con", "sin", "son", "ser", "sus", "mas", 0
    };
    int i;
    if (word == 0 || strlen(word) < 4)
    {
        return 1;
    }
    for (i = 0; stopwords[i] != 0; ++i)
    {
        if (strcmp(word, stopwords[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

static void nx_lower_ascii(char* s)
{
    size_t i;
    if (s == 0)
    {
        return;
    }
    for (i = 0; s[i] != '\0'; ++i)
    {
        s[i] = (char)tolower((unsigned char)s[i]);
    }
}

static void nx_add_concept(NxConceptCount* concepts, size_t* count, const char* word)
{
    size_t i;
    if (concepts == 0 || count == 0 || word == 0 || word[0] == '\0')
    {
        return;
    }

    if (nx_is_stopword(word))
    {
        return;
    }

    for (i = 0; i < *count; ++i)
    {
        if (strcmp(concepts[i].word, word) == 0)
        {
            concepts[i].count++;
            return;
        }
    }

    if (*count < NX_COG_MAX_CONCEPTS)
    {
        nx_copy_string(concepts[*count].word, sizeof(concepts[*count].word), word);
        concepts[*count].count = 1;
        (*count)++;
    }
}

static void nx_extract_concepts(const char* text, NxConceptCount* concepts, size_t* count)
{
    char word[64];
    size_t wi = 0;
    size_t i;

    *count = 0;
    for (i = 0; text != 0 && text[i] != '\0'; ++i)
    {
        unsigned char c = (unsigned char)text[i];
        if (isalnum(c) || c == '_' || c == '-')
        {
            if (wi + 1 < sizeof(word))
            {
                word[wi++] = (char)tolower(c);
            }
        }
        else
        {
            if (wi > 0)
            {
                word[wi] = '\0';
                nx_add_concept(concepts, count, word);
                wi = 0;
            }
        }
    }
    if (wi > 0)
    {
        word[wi] = '\0';
        nx_add_concept(concepts, count, word);
    }
}

static int nx_concept_compare(const void* a, const void* b)
{
    const NxConceptCount* ca = (const NxConceptCount*)a;
    const NxConceptCount* cb = (const NxConceptCount*)b;
    return cb->count - ca->count;
}

static size_t nx_write_chunks(FILE* out, const char* text)
{
    char sentence[768];
    size_t si = 0;
    size_t count = 0;
    size_t i;

    if (out == 0 || text == 0)
    {
        return 0;
    }

    for (i = 0; text[i] != '\0' && count < NX_COG_MAX_CHUNKS; ++i)
    {
        char c = text[i];
        if (c == '\r' || c == '\n' || c == '\t')
        {
            c = ' ';
        }

        if (si + 1 < sizeof(sentence))
        {
            sentence[si++] = c;
        }

        if (c == '.' || c == '!' || c == '?' || si > 520)
        {
            sentence[si] = '\0';
            if (si > 24)
            {
                fprintf(out, "chunk|%s\n", sentence);
                count++;
            }
            si = 0;
        }
    }

    if (si > 24 && count < NX_COG_MAX_CHUNKS)
    {
        sentence[si] = '\0';
        fprintf(out, "chunk|%s\n", sentence);
        count++;
    }

    return count;
}

static int nx_contains_word(const char* text, const char* word)
{
    char lt[1024];
    char lw[64];
    if (text == 0 || word == 0 || word[0] == '\0')
    {
        return 0;
    }
    nx_copy_string(lt, sizeof(lt), text);
    nx_copy_string(lw, sizeof(lw), word);
    nx_lower_ascii(lt);
    nx_lower_ascii(lw);
    return strstr(lt, lw) != 0;
}

static size_t nx_extract_question_terms(const char* question, char terms[][64], size_t max_terms)
{
    char word[64];
    size_t wi = 0;
    size_t count = 0;
    size_t i;

    for (i = 0; question != 0 && question[i] != '\0'; ++i)
    {
        unsigned char c = (unsigned char)question[i];
        if (isalnum(c) || c == '_' || c == '-')
        {
            if (wi + 1 < sizeof(word))
            {
                word[wi++] = (char)tolower(c);
            }
        }
        else if (wi > 0)
        {
            word[wi] = '\0';
            if (!nx_is_stopword(word) && count < max_terms)
            {
                nx_copy_string(terms[count], 64, word);
                count++;
            }
            wi = 0;
        }
    }
    if (wi > 0 && count < max_terms)
    {
        word[wi] = '\0';
        if (!nx_is_stopword(word))
        {
            nx_copy_string(terms[count], 64, word);
            count++;
        }
    }
    return count;
}


static int nx_term_is_specific(const char* term)
{
    if (term == 0) return 0;
    if (strlen(term) < 5) return 0;
    if (strcmp(term, "genexus") == 0) return 0;
    if (strcmp(term, "sirve") == 0) return 0;
    if (strcmp(term, "funciona") == 0) return 0;
    return !nx_is_stopword(term);
}

static int nx_answer_has_specific_term(const NxChunkMatch* matches, size_t match_count, char terms[][64], size_t term_count)
{
    size_t i;
    size_t j;
    int has_specific = 0;
    for (j = 0; j < term_count; ++j)
    {
        if (!nx_term_is_specific(terms[j])) continue;
        has_specific = 1;
        for (i = 0; i < match_count; ++i)
        {
            if (matches[i].text[0] != '\0' && nx_contains_word(matches[i].text, terms[j]))
            {
                return 1;
            }
        }
    }
    return !has_specific;
}

static int nx_file_exists(const char* path)
{
    FILE* f;
    if (path == 0 || path[0] == '\0') return 0;
    f = fopen(path, "rb");
    if (f == 0) return 0;
    fclose(f);
    return 1;
}

static int nx_append_file_to_topic_memory(const char* workspace_root, const char* normalized_topic, const char* source_path)
{
    char root[8] = ".";
    char topic_dir[512];
    char chunks_path[512];
    char memory_path[512];
    char sources_path[512];
    char text[NX_COG_MAX_TEXT];
    FILE* f;
    size_t bytes;
    size_t chunks;

    if (normalized_topic == 0 || source_path == 0) return 0;
    bytes = nx_read_file(source_path, text, sizeof(text));
    if (bytes == 0) return 0;

    nx_make_topic_dirs(workspace_root == 0 ? root : workspace_root, normalized_topic, topic_dir, sizeof(topic_dir));
    nx_join(chunks_path, sizeof(chunks_path), topic_dir, "chunks.txt");
    nx_join(memory_path, sizeof(memory_path), topic_dir, "memory.jsonl");
    nx_join(sources_path, sizeof(sources_path), topic_dir, "sources.txt");

    f = fopen(chunks_path, "a");
    if (f == 0) return 0;
    chunks = nx_write_chunks(f, text);
    fclose(f);

    f = fopen(sources_path, "a");
    if (f != 0)
    {
        fprintf(f, "source|%s|auto-research|bytes=%lu\n", source_path, (unsigned long)bytes);
        fclose(f);
    }

    f = fopen(memory_path, "a");
    if (f != 0)
    {
        fprintf(f, "{\"type\":\"auto-research\",\"topic\":\"%s\",\"source\":\"%s\",\"bytes\":%lu,\"chunks\":%lu}\n",
            normalized_topic, source_path, (unsigned long)bytes, (unsigned long)chunks);
        fclose(f);
    }

    return chunks > 0;
}

static int nx_try_auto_research_from_local_sources(const char* workspace_root, const char* normalized_topic, char terms[][64], size_t term_count, char* used_source, size_t used_source_size)
{
    char root[8] = ".";
    char knowledge[512];
    char cognitive[512];
    char auto_root[512];
    char topic_auto[512];
    char path[512];
    size_t i;

    if (used_source != 0 && used_source_size > 0) used_source[0] = '\0';

    nx_join(knowledge, sizeof(knowledge), workspace_root == 0 ? root : workspace_root, "Knowledge");
    nx_join(cognitive, sizeof(cognitive), knowledge, "Cognitive");
    nx_join(auto_root, sizeof(auto_root), cognitive, "AutoResearch");
    nx_join(topic_auto, sizeof(topic_auto), auto_root, normalized_topic);

    for (i = 0; i < term_count; ++i)
    {
        if (!nx_term_is_specific(terms[i])) continue;
        nx_join(path, sizeof(path), topic_auto, terms[i]);
        if (strlen(path) + 4 < sizeof(path))
        {
            strcat(path, ".txt");
        }
        if (nx_file_exists(path))
        {
            if (nx_append_file_to_topic_memory(workspace_root, normalized_topic, path))
            {
                nx_copy_string(used_source, used_source_size, path);
                return 1;
            }
        }
    }

    return 0;
}

const char* NxCognitive_StatusToString(NxCognitiveStatus status)
{
    switch (status)
    {
    case NX_COGNITIVE_OK: return "OK";
    case NX_COGNITIVE_INVALID_ARGUMENT: return "invalid argument";
    case NX_COGNITIVE_IO_ERROR: return "I/O error";
    case NX_COGNITIVE_UNSUPPORTED_INPUT: return "unsupported input";
    case NX_COGNITIVE_NOT_FOUND: return "not found";
    case NX_COGNITIVE_BUFFER_TOO_SMALL: return "buffer too small";
    default: return "unknown";
    }
}

NxCognitiveStatus NxCognitive_NormalizeTopic(const char* topic, char* output, size_t output_size)
{
    size_t i;
    size_t oi = 0;
    int last_dash = 0;

    if (topic == 0 || output == 0 || output_size == 0)
    {
        return NX_COGNITIVE_INVALID_ARGUMENT;
    }

    for (i = 0; topic[i] != '\0' && oi + 1 < output_size; ++i)
    {
        unsigned char c = (unsigned char)topic[i];
        if (isalnum(c))
        {
            output[oi++] = (char)tolower(c);
            last_dash = 0;
        }
        else if (!last_dash && oi > 0)
        {
            output[oi++] = '-';
            last_dash = 1;
        }
    }

    if (oi > 0 && output[oi - 1] == '-')
    {
        oi--;
    }
    output[oi] = '\0';

    if (output[0] == '\0')
    {
        nx_copy_string(output, output_size, "untitled");
    }

    return NX_COGNITIVE_OK;
}

NxCognitiveStatus NxCognitive_IngestFile(
    const char* workspace_root,
    const char* topic,
    const char* input_path,
    NxCognitiveIngestResult* result_out)
{
    char normalized[128];
    char root[8] = ".";
    char topic_dir[512];
    char sources_path[512];
    char chunks_path[512];
    char concepts_path[512];
    char memory_path[512];
    char text[NX_COG_MAX_TEXT];
    char sidecar[512];
    const char* read_path;
    FILE* f;
    NxConceptCount concepts[NX_COG_MAX_CONCEPTS];
    size_t concept_count = 0;
    size_t i;
    size_t bytes;
    size_t chunks = 0;
    int textual;

    if (topic == 0 || input_path == 0 || result_out == 0)
    {
        return NX_COGNITIVE_INVALID_ARGUMENT;
    }

    memset(result_out, 0, sizeof(*result_out));
    (void)NxCognitive_NormalizeTopic(topic, normalized, sizeof(normalized));
    nx_make_topic_dirs(workspace_root == 0 ? root : workspace_root, normalized, topic_dir, sizeof(topic_dir));
    nx_join(sources_path, sizeof(sources_path), topic_dir, "sources.txt");
    nx_join(chunks_path, sizeof(chunks_path), topic_dir, "chunks.txt");
    nx_join(concepts_path, sizeof(concepts_path), topic_dir, "concepts.txt");
    nx_join(memory_path, sizeof(memory_path), topic_dir, "memory.jsonl");

    textual = nx_is_textual(input_path);
    read_path = input_path;
    if (!textual && nx_is_media(input_path))
    {
        if (nx_find_sidecar(input_path, sidecar, sizeof(sidecar)))
        {
            textual = 1;
            read_path = sidecar;
        }
    }

    if (!textual)
    {
        f = fopen(sources_path, "a");
        if (f != 0)
        {
            fprintf(f, "source|%s|metadata-only|Extractor de contenido pendiente para este tipo de archivo.\n", input_path);
            fclose(f);
        }
        f = fopen(memory_path, "a");
        if (f != 0)
        {
            fprintf(f, "{\"type\":\"media\",\"topic\":\"%s\",\"path\":\"%s\",\"status\":\"metadata-only\"}\n", normalized, input_path);
            fclose(f);
        }
        nx_copy_string(result_out->topic, sizeof(result_out->topic), normalized);
        nx_copy_string(result_out->topic_dir, sizeof(result_out->topic_dir), topic_dir);
        nx_copy_string(result_out->source_path, sizeof(result_out->source_path), input_path);
        nx_copy_string(result_out->memory_path, sizeof(result_out->memory_path), memory_path);
        nx_copy_string(result_out->concepts_path, sizeof(result_out->concepts_path), concepts_path);
        nx_copy_string(result_out->chunks_path, sizeof(result_out->chunks_path), chunks_path);
        result_out->source_is_textual = 0;
        return NX_COGNITIVE_UNSUPPORTED_INPUT;
    }

    bytes = nx_read_file(read_path, text, sizeof(text));
    if (bytes == 0)
    {
        return NX_COGNITIVE_IO_ERROR;
    }

    f = fopen(sources_path, "a");
    if (f == 0)
    {
        return NX_COGNITIVE_IO_ERROR;
    }
    fprintf(f, "source|%s|textual|bytes=%lu\n", read_path, (unsigned long)bytes);
    fclose(f);

    f = fopen(chunks_path, "a");
    if (f == 0)
    {
        return NX_COGNITIVE_IO_ERROR;
    }
    chunks = nx_write_chunks(f, text);
    fclose(f);

    nx_extract_concepts(text, concepts, &concept_count);
    qsort(concepts, concept_count, sizeof(concepts[0]), nx_concept_compare);

    f = fopen(concepts_path, "w");
    if (f == 0)
    {
        return NX_COGNITIVE_IO_ERROR;
    }
    for (i = 0; i < concept_count; ++i)
    {
        fprintf(f, "%s|%d\n", concepts[i].word, concepts[i].count);
    }
    fclose(f);

    f = fopen(memory_path, "a");
    if (f == 0)
    {
        return NX_COGNITIVE_IO_ERROR;
    }
    fprintf(f, "{\"type\":\"ingest\",\"topic\":\"%s\",\"source\":\"%s\",\"bytes\":%lu,\"chunks\":%lu,\"concepts\":%lu}\n",
        normalized,
        read_path,
        (unsigned long)bytes,
        (unsigned long)chunks,
        (unsigned long)concept_count);
    fclose(f);

    nx_copy_string(result_out->topic, sizeof(result_out->topic), normalized);
    nx_copy_string(result_out->topic_dir, sizeof(result_out->topic_dir), topic_dir);
    nx_copy_string(result_out->source_path, sizeof(result_out->source_path), read_path);
    nx_copy_string(result_out->memory_path, sizeof(result_out->memory_path), memory_path);
    nx_copy_string(result_out->concepts_path, sizeof(result_out->concepts_path), concepts_path);
    nx_copy_string(result_out->chunks_path, sizeof(result_out->chunks_path), chunks_path);
    result_out->source_is_textual = 1;
    result_out->bytes_read = bytes;
    result_out->chunks_written = chunks;
    result_out->concepts_written = concept_count;
    return NX_COGNITIVE_OK;
}


static void nx_trim_line(char* s)
{
    size_t n;
    if (s == 0) return;
    while (*s != '\0' && isspace((unsigned char)*s))
    {
        memmove(s, s + 1, strlen(s));
    }
    n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1]))
    {
        s[n - 1] = '\0';
        n--;
    }
}

static void nx_canonical_text(const char* src, char* dst, size_t dst_size)
{
    size_t i;
    size_t j = 0;
    int last_space = 1;
    if (dst == 0 || dst_size == 0) return;
    dst[0] = '\0';
    if (src == 0) return;
    for (i = 0; src[i] != '\0' && j + 1 < dst_size; ++i)
    {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c))
        {
            dst[j++] = (char)tolower(c);
            last_space = 0;
        }
        else if (!last_space)
        {
            dst[j++] = ' ';
            last_space = 1;
        }
    }
    if (j > 0 && dst[j - 1] == ' ')
    {
        j--;
    }
    dst[j] = '\0';
}

static int nx_is_duplicate_or_near_duplicate(const char* a, const char* b)
{
    char ca[768];
    char cb[768];
    size_t la;
    size_t lb;
    if (a == 0 || b == 0 || a[0] == '\0' || b[0] == '\0')
    {
        return 0;
    }
    nx_canonical_text(a, ca, sizeof(ca));
    nx_canonical_text(b, cb, sizeof(cb));
    la = strlen(ca);
    lb = strlen(cb);
    if (la == 0 || lb == 0)
    {
        return 0;
    }
    if (strcmp(ca, cb) == 0)
    {
        return 1;
    }
    if (la > 24 && lb > 24)
    {
        if (la >= lb && strstr(ca, cb) != 0)
        {
            return 1;
        }
        if (lb > la && strstr(cb, ca) != 0)
        {
            return 1;
        }
    }
    return 0;
}

static int nx_match_is_unique(const NxChunkMatch* matches, size_t count, const char* text)
{
    size_t i;
    for (i = 0; i < count; ++i)
    {
        if (nx_is_duplicate_or_near_duplicate(matches[i].text, text))
        {
            return 0;
        }
    }
    return 1;
}

static void nx_insert_unique_match(NxChunkMatch* matches, size_t max_matches, const char* text, int score)
{
    size_t pos;
    size_t i;
    char clean[768];
    if (matches == 0 || max_matches == 0 || text == 0 || text[0] == '\0' || score <= 0)
    {
        return;
    }
    nx_copy_string(clean, sizeof(clean), text);
    nx_trim_line(clean);
    if (!nx_match_is_unique(matches, max_matches, clean))
    {
        return;
    }
    for (pos = 0; pos < max_matches; ++pos)
    {
        if (score > matches[pos].score)
        {
            for (i = max_matches - 1; i > pos; --i)
            {
                matches[i] = matches[i - 1];
            }
            matches[pos].score = score;
            nx_copy_string(matches[pos].text, sizeof(matches[pos].text), clean);
            return;
        }
    }
}

static void nx_append_answer_line(char* dst, size_t dst_size, size_t* used, const char* fmt, const char* value)
{
    int written;
    if (dst == 0 || dst_size == 0 || used == 0 || *used >= dst_size) return;
    written = snprintf(dst + *used, dst_size - *used, fmt, value == 0 ? "" : value);
    if (written < 0) return;
    *used += (size_t)written;
    if (*used >= dst_size) *used = dst_size - 1;
    dst[dst_size - 1] = '\0';
}


static int nx_count_matching_terms(const char* text, char terms[][64], size_t term_count)
{
    size_t i;
    int count = 0;
    for (i = 0; i < term_count; ++i)
    {
        if (nx_contains_word(text, terms[i]))
        {
            count++;
        }
    }
    return count;
}

static int nx_chunk_relevance_score(const char* text, const char* normalized_topic, char terms[][64], size_t term_count)
{
    size_t i;
    int score = 0;
    int term_hits = 0;

    if (text == 0 || text[0] == '\0')
    {
        return 0;
    }

    for (i = 0; i < term_count; ++i)
    {
        if (nx_contains_word(text, terms[i]))
        {
            term_hits++;
            score += 8;
        }
    }

    if (term_hits >= 2)
    {
        score += 18;
    }
    if (term_hits >= 3)
    {
        score += 12;
    }
    if (normalized_topic != 0 && normalized_topic[0] != '\0' && nx_contains_word(text, normalized_topic))
    {
        score += 2;
    }

    if (nx_contains_word(text, "sirve") || nx_contains_word(text, "permite") ||
        nx_contains_word(text, "representa") || nx_contains_word(text, "define") ||
        nx_contains_word(text, "usa") || nx_contains_word(text, "utiliza"))
    {
        score += 4;
    }

    return score;
}

NxCognitiveStatus NxCognitive_Ask(
    const char* workspace_root,
    const char* topic,
    const char* question,
    NxCognitiveAnswer* answer_out)
{
    char normalized[128];
    char topic_dir[512];
    char chunks_path[512];
    char answer_path[512];
    char root[8] = ".";
    char terms[16][64];
    size_t term_count;
    FILE* f;
    char line[1024];
    NxChunkMatch matches[5];
    size_t i;
    size_t candidate_count = 0;
    size_t unique_count = 0;
    int best_score = 0;
    size_t used = 0;

    if (topic == 0 || question == 0 || answer_out == 0)
    {
        return NX_COGNITIVE_INVALID_ARGUMENT;
    }

    memset(answer_out, 0, sizeof(*answer_out));
    (void)NxCognitive_NormalizeTopic(topic, normalized, sizeof(normalized));
    nx_make_topic_dirs(workspace_root == 0 ? root : workspace_root, normalized, topic_dir, sizeof(topic_dir));
    nx_join(chunks_path, sizeof(chunks_path), topic_dir, "chunks.txt");
    nx_join(answer_path, sizeof(answer_path), topic_dir, "answer.txt");

    for (i = 0; i < 5; ++i)
    {
        matches[i].text[0] = '\0';
        matches[i].score = 0;
    }

    term_count = nx_extract_question_terms(question, terms, 16);
    f = fopen(chunks_path, "rb");
    if (f == 0)
    {
        return NX_COGNITIVE_NOT_FOUND;
    }

    while (fgets(line, sizeof(line), f) != 0)
    {
        char* text = strchr(line, '|');
        int score;
        if (text == 0)
        {
            continue;
        }
        text++;
        nx_trim_line(text);
        if (text[0] == '\0')
        {
            continue;
        }

        score = nx_chunk_relevance_score(text, normalized, terms, term_count);
        if (score > 0)
        {
            candidate_count++;
            nx_insert_unique_match(matches, 5, text, score);
        }
    }
    fclose(f);

    best_score = matches[0].score;
    for (i = 0; i < 5; ++i)
    {
        if (matches[i].text[0] != '\0')
        {
            unique_count++;
        }
    }


    if (unique_count == 0 || !nx_answer_has_specific_term(matches, 5, terms, term_count))
    {
        char used_source[512];
        if (nx_try_auto_research_from_local_sources(workspace_root, normalized, terms, term_count, used_source, sizeof(used_source)))
        {
            return NxCognitive_Ask(workspace_root, topic, question, answer_out);
        }
    }

    nx_copy_string(answer_out->topic, sizeof(answer_out->topic), normalized);
    nx_copy_string(answer_out->question, sizeof(answer_out->question), question);
    nx_copy_string(answer_out->answer_path, sizeof(answer_out->answer_path), answer_path);
    answer_out->matched_chunks = unique_count;
    answer_out->confidence = best_score >= 38 ? 88 : (best_score >= 22 ? 74 : (best_score >= 8 ? 52 : 20));

    if (best_score <= 0 || !nx_answer_has_specific_term(matches, 5, terms, term_count))
    {
        (void)snprintf(answer_out->answer, sizeof(answer_out->answer),
            "No tengo evidencia suficiente sobre el concepto especifico preguntado dentro de '%s'.\n\n"
            "Accion realizada:\n"
            "- Busque una fuente local de auto-investigacion en Knowledge/Cognitive/AutoResearch/%s/.\n"
            "- No encontre una fuente suficiente para completar la respuesta.\n\n"
            "Siguiente paso:\n"
            "Agrega un archivo .txt con informacion del concepto en esa carpeta o ejecuta una investigacion especifica del tema. No voy a improvisar una respuesta con evidencia lateral.",
            topic,
            normalized);
        answer_out->confidence = 0;
    }
    else
    {
        used += (size_t)snprintf(answer_out->answer + used, sizeof(answer_out->answer) - used,
            "Respuesta priorizada desde memoria local sobre %s.\n\n", topic);
        used += (size_t)snprintf(answer_out->answer + used, sizeof(answer_out->answer) - used,
            "Pregunta: %s\n\n", question);
        used += (size_t)snprintf(answer_out->answer + used, sizeof(answer_out->answer) - used,
            "Evidencia usada:\n");

        for (i = 0; i < 5 && used + 96 < sizeof(answer_out->answer); ++i)
        {
            if (matches[i].text[0] != '\0')
            {
                char prefix[64];
                (void)snprintf(prefix, sizeof(prefix), "  %lu. [relevancia %d] %%s\n", (unsigned long)(i + 1), matches[i].score);
                nx_append_answer_line(answer_out->answer, sizeof(answer_out->answer), &used, prefix, matches[i].text);
            }
        }

        used += (size_t)snprintf(answer_out->answer + used, sizeof(answer_out->answer) - used,
            "\nResumen:\nLa respuesta se genero usando los fragmentos mas relevantes para los terminos de la pregunta. "
            "Se eliminaron fragmentos duplicados o casi duplicados antes de responder.\n\n"
            "Confianza: %d %%\nCandidatos revisados: %lu\nFragmentos unicos usados: %lu\nFuente: Knowledge/Cognitive/Topics/%s/chunks.txt",
            answer_out->confidence,
            (unsigned long)candidate_count,
            (unsigned long)unique_count,
            normalized);
    }
    answer_out->answer[sizeof(answer_out->answer) - 1] = '\0';

    f = fopen(answer_path, "wb");
    if (f != 0)
    {
        fprintf(f, "Tema: %s\nPregunta: %s\n\n%s\n", topic, question, answer_out->answer);
        fclose(f);
    }

    return NX_COGNITIVE_OK;
}

NxCognitiveStatus NxCognitive_Inspect(const char* workspace_root, const char* topic, char* output, size_t output_size)
{
    char normalized[128];
    char topic_dir[512];
    char concepts_path[512];
    char sources_path[512];
    char root[8] = ".";
    FILE* f;
    char line[256];
    size_t used = 0;
    int count = 0;

    if (topic == 0 || output == 0 || output_size == 0)
    {
        return NX_COGNITIVE_INVALID_ARGUMENT;
    }

    output[0] = '\0';
    (void)NxCognitive_NormalizeTopic(topic, normalized, sizeof(normalized));
    nx_make_topic_dirs(workspace_root == 0 ? root : workspace_root, normalized, topic_dir, sizeof(topic_dir));
    nx_join(concepts_path, sizeof(concepts_path), topic_dir, "concepts.txt");
    nx_join(sources_path, sizeof(sources_path), topic_dir, "sources.txt");

    used += (size_t)snprintf(output + used, output_size - used,
        "Tema: %s\nDirectorio: %s\n\nConceptos principales:\n",
        normalized,
        topic_dir);

    f = fopen(concepts_path, "rb");
    if (f == 0)
    {
        used += (size_t)snprintf(output + used, output_size - used,
            "  No hay conceptos registrados.\n");
    }
    else
    {
        while (fgets(line, sizeof(line), f) != 0 && count < 12 && used + 64 < output_size)
        {
            char* pipe = strchr(line, '|');
            if (pipe != 0)
            {
                *pipe = '\0';
            }
            used += (size_t)snprintf(output + used, output_size - used, "  - %s\n", line);
            count++;
        }
        fclose(f);
    }

    used += (size_t)snprintf(output + used, output_size - used, "\nFuentes:\n");
    f = fopen(sources_path, "rb");
    count = 0;
    if (f == 0)
    {
        (void)snprintf(output + used, output_size - used, "  No hay fuentes registradas.\n");
    }
    else
    {
        while (fgets(line, sizeof(line), f) != 0 && count < 8 && used + 128 < output_size)
        {
            used += (size_t)snprintf(output + used, output_size - used, "  - %s", line);
            count++;
        }
        fclose(f);
    }

    output[output_size - 1] = '\0';
    return NX_COGNITIVE_OK;
}
