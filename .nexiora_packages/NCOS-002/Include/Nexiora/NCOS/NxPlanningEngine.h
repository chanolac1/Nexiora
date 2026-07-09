#ifndef NEXIORA_NCOS_NX_PLANNING_ENGINE_H
#define NEXIORA_NCOS_NX_PLANNING_ENGINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_PLAN_MAX_ID 64
#define NX_PLAN_MAX_TEXT 256
#define NX_PLAN_MAX_PATH 512
#define NX_PLAN_MAX_STEPS 16

typedef struct NxPlanStep
{
    int index;
    char title[NX_PLAN_MAX_TEXT];
    char status[32];
} NxPlanStep;

typedef struct NxPlanStatus
{
    int exists;
    char id[NX_PLAN_MAX_ID];
    char goal[NX_PLAN_MAX_TEXT];
    char path[NX_PLAN_MAX_PATH];
    int step_count;
    int completed_count;
    NxPlanStep steps[NX_PLAN_MAX_STEPS];
} NxPlanStatus;

typedef struct NxPlanCreateResult
{
    int ok;
    char id[NX_PLAN_MAX_ID];
    char goal[NX_PLAN_MAX_TEXT];
    char path[NX_PLAN_MAX_PATH];
    int step_count;
} NxPlanCreateResult;

int NxPlanning_Create(const char* root, const char* goal, NxPlanCreateResult* out);
int NxPlanning_Status(const char* root, NxPlanStatus* out);
int NxPlanning_CompleteStep(const char* root, int step_index, NxPlanStatus* out);
int NxPlanning_AddNote(const char* root, const char* note, NxPlanStatus* out);

#ifdef __cplusplus
}
#endif

#endif
