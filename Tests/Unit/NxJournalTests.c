#include "Nexiora/NCP/Research/NxJournal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_journal_write_and_count(void)
{
    const char* path = "nx_journal_test.log";
    remove(path);

    NxJournal journal;
    if (nx_journal_open(&journal, path) != NX_OK)
        return 1;

    NxJournalEvent event = nx_journal_make_event(
        NX_JOURNAL_EVENT_EXPERIMENT_CREATED,
        "LAB-0003",
        "Journal Engine",
        "Journal test event"
    );

    if (nx_journal_write_event(&journal, &event) != NX_OK)
        return 2;

    if (nx_journal_write_event(&journal, &event) != NX_OK)
        return 3;

    size_t count = 0;
    if (nx_journal_read_count(path, &count) != NX_OK)
        return 4;

    if (count != 2)
        return 5;

    nx_journal_close(&journal);
    remove(path);
    return 0;
}

int main(void)
{
    int rc = test_journal_write_and_count();
    if (rc != 0)
    {
        printf("NxJournalTests failed: %d\n", rc);
        return rc;
    }

    printf("NxJournalTests passed\n");
    return 0;
}
