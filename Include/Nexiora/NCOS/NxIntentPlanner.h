#ifndef NEXIORA_NCOS_NX_INTENT_PLANNER_H
#define NEXIORA_NCOS_NX_INTENT_PLANNER_H

#include "Nexiora/NCOS/NxPlanningEngine.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NX_INTENT_MAX_TEXT 256
#define NX_INTENT_MAX_KIND 64
#define NX_INTENT_MAX_TARGET 128

typedef struct NxIntentPlanResult
{
    int ok;
    char intent[NX_INTENT_MAX_KIND];
    char target[NX_INTENT_MAX_TARGET];
    char normalized_goal[NX_INTENT_MAX_TEXT];
    NxPlanCreateResult plan;
} NxIntentPlanResult;

int NxIntentPlanner_CreatePlan(const char* root, const char* request, NxIntentPlanResult* out);
int NxIntentPlanner_Classify(const char* request, char* intent, int intent_size, char* target, int target_size);

#ifdef __cplusplus
}
#endif

#endif
