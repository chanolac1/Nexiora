#ifndef NEXIORA_RESEARCH_NX_DISCOVERY_ENGINE_H
#define NEXIORA_RESEARCH_NX_DISCOVERY_ENGINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxDiscoveryStatus
{
    NX_DISCOVERY_STATUS_OK = 0,
    NX_DISCOVERY_STATUS_INVALID_ARGUMENT = 1,
    NX_DISCOVERY_STATUS_CAPACITY_EXCEEDED = 2,
    NX_DISCOVERY_STATUS_NOT_FOUND = 3,
    NX_DISCOVERY_STATUS_DUPLICATE_ID = 4
} NxDiscoveryStatus;

typedef enum NxExperimentKind
{
    NX_EXPERIMENT_KIND_UNKNOWN = 0,
    NX_EXPERIMENT_KIND_BENCHMARK = 1,
    NX_EXPERIMENT_KIND_CORRECTNESS = 2,
    NX_EXPERIMENT_KIND_RESEARCH = 3,
    NX_EXPERIMENT_KIND_PROMOTION_CANDIDATE = 4
} NxExperimentKind;

typedef enum NxExperimentState
{
    NX_EXPERIMENT_STATE_UNKNOWN = 0,
    NX_EXPERIMENT_STATE_REGISTERED = 1,
    NX_EXPERIMENT_STATE_READY = 2,
    NX_EXPERIMENT_STATE_DISABLED = 3,
    NX_EXPERIMENT_STATE_DEPRECATED = 4
} NxExperimentState;

typedef struct NxExperimentDescriptor
{
    const char* id;
    const char* name;
    const char* component;
    const char* owner;
    NxExperimentKind kind;
    NxExperimentState state;
    unsigned int priority;
} NxExperimentDescriptor;

typedef struct NxDiscoveryEngine
{
    NxExperimentDescriptor* descriptors;
    size_t capacity;
    size_t count;
} NxDiscoveryEngine;

typedef int (*NxDiscoveryPredicate)(
    const NxExperimentDescriptor* descriptor,
    void* user_data);

NxDiscoveryStatus NxDiscoveryEngine_Init(
    NxDiscoveryEngine* engine,
    NxExperimentDescriptor* storage,
    size_t capacity);

void NxDiscoveryEngine_Reset(NxDiscoveryEngine* engine);

NxDiscoveryStatus NxDiscoveryEngine_Register(
    NxDiscoveryEngine* engine,
    const NxExperimentDescriptor* descriptor);

size_t NxDiscoveryEngine_Count(const NxDiscoveryEngine* engine);

NxDiscoveryStatus NxDiscoveryEngine_Get(
    const NxDiscoveryEngine* engine,
    size_t index,
    const NxExperimentDescriptor** descriptor_out);

NxDiscoveryStatus NxDiscoveryEngine_FindById(
    const NxDiscoveryEngine* engine,
    const char* id,
    const NxExperimentDescriptor** descriptor_out);

size_t NxDiscoveryEngine_Discover(
    const NxDiscoveryEngine* engine,
    NxDiscoveryPredicate predicate,
    void* user_data,
    const NxExperimentDescriptor** results,
    size_t result_capacity);

int NxDiscoveryPredicate_All(
    const NxExperimentDescriptor* descriptor,
    void* user_data);

int NxDiscoveryPredicate_Ready(
    const NxExperimentDescriptor* descriptor,
    void* user_data);

int NxDiscoveryPredicate_Component(
    const NxExperimentDescriptor* descriptor,
    void* user_data);

#ifdef __cplusplus
}
#endif

#endif
