#ifndef NEXIORA_NCP_RESEARCH_NXJOURNAL_H
#define NEXIORA_NCP_RESEARCH_NXJOURNAL_H

#include <stddef.h>
#include <stdint.h>
#include "Nexiora/NCP/Common/NxResult.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NX_JOURNAL_EVENT_NAME_MAX 64
#define NX_JOURNAL_EXPERIMENT_ID_MAX 32
#define NX_JOURNAL_MESSAGE_MAX 256
#define NX_JOURNAL_PATH_MAX 260

typedef enum NxJournalEventType
{
    NX_JOURNAL_EVENT_UNKNOWN = 0,
    NX_JOURNAL_EVENT_EXPERIMENT_CREATED = 1,
    NX_JOURNAL_EVENT_EXPERIMENT_STARTED = 2,
    NX_JOURNAL_EVENT_BENCHMARK_COMPLETED = 3,
    NX_JOURNAL_EVENT_EVIDENCE_GENERATED = 4,
    NX_JOURNAL_EVENT_EXPERIMENT_APPROVED = 5,
    NX_JOURNAL_EVENT_EXPERIMENT_ARCHIVED = 6
} NxJournalEventType;

typedef struct NxJournalEvent
{
    uint64_t timestamp_unix;
    NxJournalEventType type;
    char experiment_id[NX_JOURNAL_EXPERIMENT_ID_MAX];
    char name[NX_JOURNAL_EVENT_NAME_MAX];
    char message[NX_JOURNAL_MESSAGE_MAX];
} NxJournalEvent;

typedef struct NxJournal
{
    char path[NX_JOURNAL_PATH_MAX];
    size_t events_written;
} NxJournal;

const char* nx_journal_event_type_to_string(NxJournalEventType type);

NxResult nx_journal_open(NxJournal* journal, const char* path);
NxResult nx_journal_write_event(NxJournal* journal, const NxJournalEvent* event);
NxResult nx_journal_read_count(const char* path, size_t* out_count);
NxResult nx_journal_close(NxJournal* journal);

NxJournalEvent nx_journal_make_event(
    NxJournalEventType type,
    const char* experiment_id,
    const char* name,
    const char* message
);

#ifdef __cplusplus
}
#endif

#endif
