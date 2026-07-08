#include "Nexiora/Research/NxManifest.h"
#include "Nexiora/NCP/String/NxString.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char* nx_trim(char* text) {
    if (text == NULL) return NULL;
    while (*text && isspace((unsigned char)*text)) ++text;
    char* end = text + strlen(text);
    while (end > text && isspace((unsigned char)*(end - 1))) --end;
    *end = '\0';
    return text;
}

static NxResult nx_copy_value(char* destination, size_t destination_size, const char* value) {
    if (destination == NULL || value == NULL || destination_size == 0) return NX_ERROR_ARGUMENT;
    return nx_string_copy(destination, destination_size, value);
}

static NxExperimentStatus nx_status_from_string(const char* status) {
    if (status == NULL) return NX_EXPERIMENT_STATUS_DRAFT;
    if (strcmp(status, "Prepared") == 0) return NX_EXPERIMENT_STATUS_PREPARED;
    if (strcmp(status, "Running") == 0) return NX_EXPERIMENT_STATUS_RUNNING;
    if (strcmp(status, "Measured") == 0) return NX_EXPERIMENT_STATUS_MEASURED;
    if (strcmp(status, "Validated") == 0) return NX_EXPERIMENT_STATUS_VALIDATED;
    if (strcmp(status, "Approved") == 0) return NX_EXPERIMENT_STATUS_APPROVED;
    if (strcmp(status, "Integrated") == 0) return NX_EXPERIMENT_STATUS_INTEGRATED;
    if (strcmp(status, "Archived") == 0) return NX_EXPERIMENT_STATUS_ARCHIVED;
    if (strcmp(status, "Rejected") == 0) return NX_EXPERIMENT_STATUS_REJECTED;
    return NX_EXPERIMENT_STATUS_DRAFT;
}

NxResult nx_manifest_write_experiment(const char* path, const NxExperiment* experiment) {
    return nx_experiment_write_manifest(experiment, path);
}

NxResult nx_manifest_read_experiment(const char* path, NxExperiment* experiment) {
    if (path == NULL || experiment == NULL) return NX_ERROR_ARGUMENT;

    FILE* file = fopen(path, "rb");
    if (file == NULL) return NX_ERROR_IO;

    char id[NX_EXPERIMENT_ID_MAX] = {0};
    char title[NX_EXPERIMENT_TITLE_MAX] = {0};
    char author[NX_EXPERIMENT_AUTHOR_MAX] = {0};
    char module[NX_EXPERIMENT_MODULE_MAX] = {0};
    char target[NX_EXPERIMENT_MODULE_MAX] = {0};
    char hypothesis[NX_EXPERIMENT_TEXT_MAX] = {0};
    NxExperimentStatus status = NX_EXPERIMENT_STATUS_DRAFT;

    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        char* colon = strchr(line, ':');
        if (colon == NULL) continue;
        *colon = '\0';
        char* key = nx_trim(line);
        char* value = nx_trim(colon + 1);
        if (strcmp(key, "Id") == 0) nx_copy_value(id, sizeof(id), value);
        else if (strcmp(key, "Title") == 0) nx_copy_value(title, sizeof(title), value);
        else if (strcmp(key, "Author") == 0) nx_copy_value(author, sizeof(author), value);
        else if (strcmp(key, "Module") == 0) nx_copy_value(module, sizeof(module), value);
        else if (strcmp(key, "Target") == 0) nx_copy_value(target, sizeof(target), value);
        else if (strcmp(key, "Hypothesis") == 0) nx_copy_value(hypothesis, sizeof(hypothesis), value);
        else if (strcmp(key, "Status") == 0) status = nx_status_from_string(value);
    }

    fclose(file);

    if (id[0] == '\0' || title[0] == '\0' || author[0] == '\0' || module[0] == '\0' || hypothesis[0] == '\0') {
        return NX_ERROR_ARGUMENT;
    }
    if (target[0] == '\0') {
        nx_copy_value(target, sizeof(target), "Research");
    }

    NxResult result = nx_experiment_initialize(experiment, id, title, author, module, hypothesis, target);
    if (result != NX_OK) return result;
    experiment->status = status;
    return NX_OK;
}
