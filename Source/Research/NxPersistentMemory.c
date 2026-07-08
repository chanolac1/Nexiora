#include "Nexiora/Research/NxPersistentMemory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define nx_mkdir(path) _mkdir(path)
#else
#include <sys/stat.h>
#define nx_mkdir(path) mkdir(path, 0777)
#endif

#define NX_MEMORY_INITIAL_CAPACITY ((size_t)16)

static void nx_memory_copy(char* dst, size_t dst_size, const char* src)
{
    size_t i = 0;
    if (dst == 0 || dst_size == 0) return;
    if (src == 0) { dst[0] = '\0'; return; }
    while (i + 1 < dst_size && src[i] != '\0') { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

static int nx_memory_text_equals(const char* a, const char* b)
{
    return a != 0 && b != 0 && strcmp(a, b) == 0;
}

static NxPersistentMemoryStatus nx_memory_ensure_capacity(NxPersistentMemory* memory, size_t required)
{
    size_t new_capacity;
    NxMemoryRecord* new_records;
    if (memory == 0) return NX_PERSISTENT_MEMORY_INVALID_ARGUMENT;
    if (memory->record_capacity >= required) return NX_PERSISTENT_MEMORY_OK;
    new_capacity = memory->record_capacity == 0 ? NX_MEMORY_INITIAL_CAPACITY : memory->record_capacity;
    while (new_capacity < required) new_capacity *= 2;
    new_records = (NxMemoryRecord*)realloc(memory->records, new_capacity * sizeof(NxMemoryRecord));
    if (new_records == 0) return NX_PERSISTENT_MEMORY_OUT_OF_MEMORY;
    memory->records = new_records;
    memory->record_capacity = new_capacity;
    return NX_PERSISTENT_MEMORY_OK;
}

static void nx_memory_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    size_t used = 0;
    if (dst == 0 || dst_size == 0) return;
    dst[0] = '\0';
    if (a != 0) {
        while (used + 1 < dst_size && a[used] != '\0') { dst[used] = a[used]; used++; }
    }
    if (used + 1 < dst_size && used > 0 && dst[used - 1] != '\\' && dst[used - 1] != '/') dst[used++] = '\\';
    if (b != 0) {
        size_t i = 0;
        while (used + 1 < dst_size && b[i] != '\0') { dst[used++] = b[i++]; }
    }
    dst[used] = '\0';
}

static int nx_memory_extract_json_string(const char* line, const char* key, char* out, size_t out_size)
{
    char pattern[64];
    const char* start;
    const char* end;
    size_t len;
    snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
    start = strstr(line, pattern);
    if (start == 0) return 0;
    start += strlen(pattern);
    end = strchr(start, '"');
    if (end == 0) return 0;
    len = (size_t)(end - start);
    if (len >= out_size) len = out_size - 1;
    memcpy(out, start, len);
    out[len] = '\0';
    return 1;
}

static int nx_memory_extract_json_int(const char* line, const char* key, int* out)
{
    char pattern[64];
    const char* start;
    snprintf(pattern, sizeof(pattern), "\"%s\":", key);
    start = strstr(line, pattern);
    if (start == 0 || out == 0) return 0;
    start += strlen(pattern);
    *out = atoi(start);
    return 1;
}

NxPersistentMemoryStatus NxPersistentMemory_Init(NxPersistentMemory* memory)
{
    if (memory == 0) return NX_PERSISTENT_MEMORY_INVALID_ARGUMENT;
    memory->records = 0;
    memory->record_count = 0;
    memory->record_capacity = 0;
    memory->next_id = 1;
    return NX_PERSISTENT_MEMORY_OK;
}

void NxPersistentMemory_Shutdown(NxPersistentMemory* memory)
{
    if (memory == 0) return;
    free(memory->records);
    memory->records = 0;
    memory->record_count = 0;
    memory->record_capacity = 0;
    memory->next_id = 1;
}

void NxPersistentMemory_Clear(NxPersistentMemory* memory)
{
    if (memory == 0) return;
    memory->record_count = 0;
    memory->next_id = 1;
}

NxPersistentMemoryStatus NxPersistentMemory_AddRecord(
    NxPersistentMemory* memory,
    NxMemoryRecordType type,
    const char* title,
    const char* body,
    const char* source,
    const char* timestamp,
    int confidence,
    uint32_t* record_id_out)
{
    NxPersistentMemoryStatus status;
    NxMemoryRecord* record;
    if (memory == 0 || title == 0 || body == 0 || source == 0 || timestamp == 0 || record_id_out == 0)
        return NX_PERSISTENT_MEMORY_INVALID_ARGUMENT;
    if (confidence < 0 || confidence > 100) return NX_PERSISTENT_MEMORY_INVALID_CONFIDENCE;
    status = nx_memory_ensure_capacity(memory, memory->record_count + 1);
    if (status != NX_PERSISTENT_MEMORY_OK) return status;
    record = &memory->records[memory->record_count];
    record->id = memory->next_id++;
    record->type = type;
    nx_memory_copy(record->title, sizeof(record->title), title);
    nx_memory_copy(record->body, sizeof(record->body), body);
    nx_memory_copy(record->source, sizeof(record->source), source);
    nx_memory_copy(record->timestamp, sizeof(record->timestamp), timestamp);
    record->confidence = confidence;
    *record_id_out = record->id;
    memory->record_count++;
    return NX_PERSISTENT_MEMORY_OK;
}

size_t NxPersistentMemory_GetRecordCount(const NxPersistentMemory* memory)
{
    return memory == 0 ? 0 : memory->record_count;
}

size_t NxPersistentMemory_CountByType(const NxPersistentMemory* memory, NxMemoryRecordType type)
{
    size_t i, count = 0;
    if (memory == 0) return 0;
    for (i = 0; i < memory->record_count; i++) if (memory->records[i].type == type) count++;
    return count;
}

const NxMemoryRecord* NxPersistentMemory_FindById(const NxPersistentMemory* memory, uint32_t id)
{
    size_t i;
    if (memory == 0) return 0;
    for (i = 0; i < memory->record_count; i++) if (memory->records[i].id == id) return &memory->records[i];
    return 0;
}

const NxMemoryRecord* NxPersistentMemory_FindByTitle(const NxPersistentMemory* memory, const char* title)
{
    size_t i;
    if (memory == 0 || title == 0) return 0;
    for (i = 0; i < memory->record_count; i++) if (nx_memory_text_equals(memory->records[i].title, title)) return &memory->records[i];
    return 0;
}

NxPersistentMemoryStatus NxPersistentMemory_GetSummary(
    const NxPersistentMemory* memory,
    NxPersistentMemorySummary* summary_out)
{
    size_t i;
    int total_confidence = 0;
    if (memory == 0 || summary_out == 0) return NX_PERSISTENT_MEMORY_INVALID_ARGUMENT;
    memset(summary_out, 0, sizeof(*summary_out));
    for (i = 0; i < memory->record_count; i++) {
        const NxMemoryRecord* r = &memory->records[i];
        if (r->type == NX_MEMORY_RECORD_FACT) summary_out->facts++;
        else if (r->type == NX_MEMORY_RECORD_DECISION) summary_out->decisions++;
        else if (r->type == NX_MEMORY_RECORD_HYPOTHESIS) summary_out->hypotheses++;
        else if (r->type == NX_MEMORY_RECORD_CONCEPT) summary_out->concepts++;
        total_confidence += r->confidence;
    }
    summary_out->average_confidence = memory->record_count == 0 ? 0 : total_confidence / (int)memory->record_count;
    return NX_PERSISTENT_MEMORY_OK;
}

NxPersistentMemoryStatus NxPersistentMemory_SaveJsonl(
    const NxPersistentMemory* memory,
    const char* path)
{
    FILE* file;
    size_t i;
    if (memory == 0 || path == 0) return NX_PERSISTENT_MEMORY_INVALID_ARGUMENT;
    file = fopen(path, "w");
    if (file == 0) return NX_PERSISTENT_MEMORY_IO_ERROR;
    for (i = 0; i < memory->record_count; i++) {
        const NxMemoryRecord* r = &memory->records[i];
        fprintf(file, "{\"id\":%u,\"type\":%d,\"title\":\"%s\",\"body\":\"%s\",\"source\":\"%s\",\"timestamp\":\"%s\",\"confidence\":%d}\n",
            r->id, (int)r->type, r->title, r->body, r->source, r->timestamp, r->confidence);
    }
    fclose(file);
    return NX_PERSISTENT_MEMORY_OK;
}

NxPersistentMemoryStatus NxPersistentMemory_LoadJsonl(
    NxPersistentMemory* memory,
    const char* path)
{
    FILE* file;
    char line[1024];
    if (memory == 0 || path == 0) return NX_PERSISTENT_MEMORY_INVALID_ARGUMENT;
    file = fopen(path, "r");
    if (file == 0) return NX_PERSISTENT_MEMORY_IO_ERROR;
    NxPersistentMemory_Clear(memory);
    while (fgets(line, sizeof(line), file) != 0) {
        int type = 0, confidence = 0;
        uint32_t id = 0;
        char title[96], body[256], source[128], timestamp[32];
        uint32_t generated_id = 0;
        if (!nx_memory_extract_json_int(line, "id", (int*)&id)) continue;
        if (!nx_memory_extract_json_int(line, "type", &type)) continue;
        if (!nx_memory_extract_json_string(line, "title", title, sizeof(title))) continue;
        if (!nx_memory_extract_json_string(line, "body", body, sizeof(body))) continue;
        if (!nx_memory_extract_json_string(line, "source", source, sizeof(source))) continue;
        if (!nx_memory_extract_json_string(line, "timestamp", timestamp, sizeof(timestamp))) continue;
        if (!nx_memory_extract_json_int(line, "confidence", &confidence)) continue;
        if (NxPersistentMemory_AddRecord(memory, (NxMemoryRecordType)type, title, body, source, timestamp, confidence, &generated_id) != NX_PERSISTENT_MEMORY_OK) continue;
        memory->records[memory->record_count - 1].id = id;
        if (memory->next_id <= id) memory->next_id = id + 1;
    }
    fclose(file);
    return NX_PERSISTENT_MEMORY_OK;
}

NxPersistentMemoryStatus NxPersistentMemory_SeedFromFirstAutonomousExecution(
    const char* repository_root,
    char* memory_path_out,
    size_t memory_path_out_size)
{
    NxPersistentMemory memory;
    NxPersistentMemoryStatus status;
    uint32_t id = 0;
    char knowledge_path[260];
    char memory_dir[260];
    char path[260];
    const char* root = repository_root == 0 ? "." : repository_root;
    nx_memory_join(knowledge_path, sizeof(knowledge_path), root, "Knowledge");
    nx_memory_join(memory_dir, sizeof(memory_dir), knowledge_path, "Memory");
    (void)nx_mkdir(knowledge_path);
    (void)nx_mkdir(memory_dir);
    nx_memory_join(path, sizeof(path), memory_dir, "memory.jsonl");

    status = NxPersistentMemory_Init(&memory);
    if (status != NX_PERSISTENT_MEMORY_OK) return status;
    (void)NxPersistentMemory_AddRecord(&memory, NX_MEMORY_RECORD_FACT, "Primera ejecucion autonoma", "Nexiora ejecuto una sesion autonoma completa y genero evidencia, grafo y dashboard.", "EPIC-0005/EPIC-0006", "2026-07-08", 95, &id);
    (void)NxPersistentMemory_AddRecord(&memory, NX_MEMORY_RECORD_DECISION, "Runtime protegido", "La promocion al Runtime permanece bloqueada hasta aprobacion humana.", "BOOK", "2026-07-08", 100, &id);
    (void)NxPersistentMemory_AddRecord(&memory, NX_MEMORY_RECORD_CONCEPT, "Memoria persistente", "La memoria guarda hechos, decisiones, hipotesis y conceptos para consultas futuras.", "EPIC-0008", "2026-07-08", 90, &id);
    (void)NxPersistentMemory_AddRecord(&memory, NX_MEMORY_RECORD_HYPOTHESIS, "Nexiora puede recordar", "Si la memoria persistente se mantiene entre ejecuciones, Nexiora podra explicar que aprendio antes.", "EPIC-0008", "2026-07-08", 82, &id);
    status = NxPersistentMemory_SaveJsonl(&memory, path);
    NxPersistentMemory_Shutdown(&memory);
    if (status != NX_PERSISTENT_MEMORY_OK) return status;
    nx_memory_copy(memory_path_out, memory_path_out_size, path);
    return NX_PERSISTENT_MEMORY_OK;
}

const char* NxPersistentMemory_RecordTypeToString(NxMemoryRecordType type)
{
    switch (type) {
    case NX_MEMORY_RECORD_FACT: return "fact";
    case NX_MEMORY_RECORD_DECISION: return "decision";
    case NX_MEMORY_RECORD_HYPOTHESIS: return "hypothesis";
    case NX_MEMORY_RECORD_CONCEPT: return "concept";
    default: return "unknown";
    }
}

const char* NxPersistentMemory_StatusToString(NxPersistentMemoryStatus status)
{
    switch (status) {
    case NX_PERSISTENT_MEMORY_OK: return "ok";
    case NX_PERSISTENT_MEMORY_INVALID_ARGUMENT: return "invalid argument";
    case NX_PERSISTENT_MEMORY_OUT_OF_MEMORY: return "out of memory";
    case NX_PERSISTENT_MEMORY_IO_ERROR: return "I/O error";
    case NX_PERSISTENT_MEMORY_NOT_FOUND: return "not found";
    case NX_PERSISTENT_MEMORY_INVALID_CONFIDENCE: return "invalid confidence";
    default: return "unknown";
    }
}
