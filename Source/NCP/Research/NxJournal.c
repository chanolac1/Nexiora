#include "Nexiora/NCP/Research/NxJournal.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static void nx_journal_copy(char* dst, size_t dst_size, const char* src)
{
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    size_t i = 0;
    for (; i + 1 < dst_size && src[i] != '\0'; ++i)
        dst[i] = src[i];
    dst[i] = '\0';
}

const char* nx_journal_event_type_to_string(NxJournalEventType type)
{
    switch (type)
    {
        case NX_JOURNAL_EVENT_EXPERIMENT_CREATED: return "ExperimentCreated";
        case NX_JOURNAL_EVENT_EXPERIMENT_STARTED: return "ExperimentStarted";
        case NX_JOURNAL_EVENT_BENCHMARK_COMPLETED: return "BenchmarkCompleted";
        case NX_JOURNAL_EVENT_EVIDENCE_GENERATED: return "EvidenceGenerated";
        case NX_JOURNAL_EVENT_EXPERIMENT_APPROVED: return "ExperimentApproved";
        case NX_JOURNAL_EVENT_EXPERIMENT_ARCHIVED: return "ExperimentArchived";
        default: return "Unknown";
    }
}

NxJournalEvent nx_journal_make_event(
    NxJournalEventType type,
    const char* experiment_id,
    const char* name,
    const char* message
)
{
    NxJournalEvent event;
    event.timestamp_unix = (uint64_t)time(NULL);
    event.type = type;
    nx_journal_copy(event.experiment_id, sizeof(event.experiment_id), experiment_id);
    nx_journal_copy(event.name, sizeof(event.name), name);
    nx_journal_copy(event.message, sizeof(event.message), message);
    return event;
}

NxResult nx_journal_open(NxJournal* journal, const char* path)
{
    if (!journal || !path || path[0] == '\0')
        return NX_ERROR_ARGUMENT;

    nx_journal_copy(journal->path, sizeof(journal->path), path);
    journal->events_written = 0;

    FILE* file = fopen(path, "ab");
    if (!file)
        return NX_ERROR_IO;

    fclose(file);
    return NX_OK;
}

NxResult nx_journal_write_event(NxJournal* journal, const NxJournalEvent* event)
{
    if (!journal || !event || journal->path[0] == '\0')
        return NX_ERROR_ARGUMENT;

    FILE* file = fopen(journal->path, "ab");
    if (!file)
        return NX_ERROR_IO;

    int written = fprintf(
        file,
        "%llu|%s|%s|%s|%s\n",
        (unsigned long long)event->timestamp_unix,
        nx_journal_event_type_to_string(event->type),
        event->experiment_id,
        event->name,
        event->message
    );

    fclose(file);

    if (written <= 0)
        return NX_ERROR_IO;

    journal->events_written++;
    return NX_OK;
}

NxResult nx_journal_read_count(const char* path, size_t* out_count)
{
    if (!path || !out_count)
        return NX_ERROR_ARGUMENT;

    *out_count = 0;

    FILE* file = fopen(path, "rb");
    if (!file)
        return NX_ERROR_IO;

    int ch = 0;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
            (*out_count)++;
    }

    fclose(file);
    return NX_OK;
}

NxResult nx_journal_close(NxJournal* journal)
{
    if (!journal)
        return NX_ERROR_ARGUMENT;

    journal->path[0] = '\0';
    journal->events_written = 0;
    return NX_OK;
}

