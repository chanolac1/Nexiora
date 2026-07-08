#include "Nexiora/Research/NxExperiment.h"
#include "Nexiora/NCP/String/NxString.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static uint64_t nx_research_now_unix_seconds(void) {
    return (uint64_t)time(NULL);
}

static NxResult nx_copy_field(char* destination, size_t destination_size, const char* source) {
    if (source == NULL || destination == NULL || destination_size == 0) {
        return NX_ERROR_ARGUMENT;
    }
    return nx_string_copy(destination, destination_size, source);
}

const char* nx_experiment_status_to_string(NxExperimentStatus status) {
    switch (status) {
        case NX_EXPERIMENT_STATUS_DRAFT: return "Draft";
        case NX_EXPERIMENT_STATUS_PREPARED: return "Prepared";
        case NX_EXPERIMENT_STATUS_RUNNING: return "Running";
        case NX_EXPERIMENT_STATUS_MEASURED: return "Measured";
        case NX_EXPERIMENT_STATUS_VALIDATED: return "Validated";
        case NX_EXPERIMENT_STATUS_APPROVED: return "Approved";
        case NX_EXPERIMENT_STATUS_INTEGRATED: return "Integrated";
        case NX_EXPERIMENT_STATUS_ARCHIVED: return "Archived";
        case NX_EXPERIMENT_STATUS_REJECTED: return "Rejected";
        default: return "Unknown";
    }
}

NxResult nx_experiment_initialize(NxExperiment* experiment,
                                  const char* id,
                                  const char* title,
                                  const char* author,
                                  const char* module,
                                  const char* hypothesis,
                                  const char* target) {
    if (experiment == NULL || id == NULL || title == NULL || author == NULL || module == NULL || hypothesis == NULL || target == NULL) {
        return NX_ERROR_ARGUMENT;
    }

    memset(experiment, 0, sizeof(*experiment));

    if (nx_copy_field(experiment->id, sizeof(experiment->id), id) != NX_OK) return NX_ERROR_ARGUMENT;
    if (nx_copy_field(experiment->title, sizeof(experiment->title), title) != NX_OK) return NX_ERROR_ARGUMENT;
    if (nx_copy_field(experiment->author, sizeof(experiment->author), author) != NX_OK) return NX_ERROR_ARGUMENT;
    if (nx_copy_field(experiment->module, sizeof(experiment->module), module) != NX_OK) return NX_ERROR_ARGUMENT;
    if (nx_copy_field(experiment->hypothesis, sizeof(experiment->hypothesis), hypothesis) != NX_OK) return NX_ERROR_ARGUMENT;
    if (nx_copy_field(experiment->target, sizeof(experiment->target), target) != NX_OK) return NX_ERROR_ARGUMENT;

    experiment->status = NX_EXPERIMENT_STATUS_DRAFT;
    experiment->created_unix_seconds = nx_research_now_unix_seconds();
    experiment->last_execution_unix_seconds = 0;
    return NX_OK;
}

int nx_experiment_transition_is_valid(NxExperimentStatus current_status, NxExperimentStatus next_status) {
    if (current_status == next_status) {
        return 1;
    }

    switch (current_status) {
        case NX_EXPERIMENT_STATUS_DRAFT:
            return next_status == NX_EXPERIMENT_STATUS_PREPARED || next_status == NX_EXPERIMENT_STATUS_ARCHIVED || next_status == NX_EXPERIMENT_STATUS_REJECTED;
        case NX_EXPERIMENT_STATUS_PREPARED:
            return next_status == NX_EXPERIMENT_STATUS_RUNNING || next_status == NX_EXPERIMENT_STATUS_ARCHIVED || next_status == NX_EXPERIMENT_STATUS_REJECTED;
        case NX_EXPERIMENT_STATUS_RUNNING:
            return next_status == NX_EXPERIMENT_STATUS_MEASURED || next_status == NX_EXPERIMENT_STATUS_REJECTED;
        case NX_EXPERIMENT_STATUS_MEASURED:
            return next_status == NX_EXPERIMENT_STATUS_VALIDATED || next_status == NX_EXPERIMENT_STATUS_ARCHIVED || next_status == NX_EXPERIMENT_STATUS_REJECTED;
        case NX_EXPERIMENT_STATUS_VALIDATED:
            return next_status == NX_EXPERIMENT_STATUS_APPROVED || next_status == NX_EXPERIMENT_STATUS_ARCHIVED || next_status == NX_EXPERIMENT_STATUS_REJECTED;
        case NX_EXPERIMENT_STATUS_APPROVED:
            return next_status == NX_EXPERIMENT_STATUS_INTEGRATED || next_status == NX_EXPERIMENT_STATUS_ARCHIVED;
        case NX_EXPERIMENT_STATUS_INTEGRATED:
            return next_status == NX_EXPERIMENT_STATUS_ARCHIVED;
        case NX_EXPERIMENT_STATUS_ARCHIVED:
        case NX_EXPERIMENT_STATUS_REJECTED:
        default:
            return 0;
    }
}

NxResult nx_experiment_transition(NxExperiment* experiment, NxExperimentStatus next_status) {
    if (experiment == NULL) {
        return NX_ERROR_ARGUMENT;
    }
    if (!nx_experiment_transition_is_valid(experiment->status, next_status)) {
        return NX_ERROR_UNSUPPORTED;
    }
    experiment->status = next_status;
    if (next_status == NX_EXPERIMENT_STATUS_RUNNING || next_status == NX_EXPERIMENT_STATUS_MEASURED) {
        experiment->last_execution_unix_seconds = nx_research_now_unix_seconds();
    }
    return NX_OK;
}

uint32_t nx_experiment_hash(const NxExperiment* experiment) {
    if (experiment == NULL) {
        return 0;
    }

    const unsigned char* data = (const unsigned char*)experiment;
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < sizeof(*experiment); ++i) {
        hash ^= (uint32_t)data[i];
        hash *= 16777619u;
    }
    return hash;
}

NxResult nx_experiment_write_manifest(const NxExperiment* experiment, const char* path) {
    if (experiment == NULL || path == NULL) {
        return NX_ERROR_ARGUMENT;
    }

    FILE* file = fopen(path, "wb");
    if (file == NULL) {
        return NX_ERROR_IO;
    }

    fprintf(file, "Experiment\n{\n");
    fprintf(file, "    Id         : %s\n", experiment->id);
    fprintf(file, "    Title      : %s\n", experiment->title);
    fprintf(file, "    Author     : %s\n", experiment->author);
    fprintf(file, "    Module     : %s\n", experiment->module);
    fprintf(file, "    Target     : %s\n", experiment->target);
    fprintf(file, "    Status     : %s\n", nx_experiment_status_to_string(experiment->status));
    fprintf(file, "    Hash       : %u\n", nx_experiment_hash(experiment));
    fprintf(file, "    Hypothesis : %s\n", experiment->hypothesis);
    fprintf(file, "}\n");

    fclose(file);
    return NX_OK;
}
