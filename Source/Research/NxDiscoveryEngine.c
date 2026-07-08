#include "Nexiora/Research/NxDiscoveryEngine.h"

#include <string.h>

static int NxDiscoveryStringEquals(const char* left, const char* right)
{
    if (left == 0 || right == 0)
    {
        return 0;
    }

    return strcmp(left, right) == 0;
}

static int NxDiscoveryDescriptorIsValid(const NxExperimentDescriptor* descriptor)
{
    if (descriptor == 0)
    {
        return 0;
    }

    if (descriptor->id == 0 || descriptor->id[0] == '\0')
    {
        return 0;
    }

    if (descriptor->name == 0 || descriptor->name[0] == '\0')
    {
        return 0;
    }

    if (descriptor->component == 0 || descriptor->component[0] == '\0')
    {
        return 0;
    }

    return 1;
}

NxDiscoveryStatus NxDiscoveryEngine_Init(
    NxDiscoveryEngine* engine,
    NxExperimentDescriptor* storage,
    size_t capacity)
{
    if (engine == 0 || storage == 0 || capacity == 0)
    {
        return NX_DISCOVERY_STATUS_INVALID_ARGUMENT;
    }

    engine->descriptors = storage;
    engine->capacity = capacity;
    engine->count = 0;

    return NX_DISCOVERY_STATUS_OK;
}

void NxDiscoveryEngine_Reset(NxDiscoveryEngine* engine)
{
    if (engine == 0)
    {
        return;
    }

    engine->count = 0;
}

NxDiscoveryStatus NxDiscoveryEngine_Register(
    NxDiscoveryEngine* engine,
    const NxExperimentDescriptor* descriptor)
{
    size_t index = 0;

    if (engine == 0 || engine->descriptors == 0 ||
        !NxDiscoveryDescriptorIsValid(descriptor))
    {
        return NX_DISCOVERY_STATUS_INVALID_ARGUMENT;
    }

    if (engine->count >= engine->capacity)
    {
        return NX_DISCOVERY_STATUS_CAPACITY_EXCEEDED;
    }

    for (index = 0; index < engine->count; ++index)
    {
        if (NxDiscoveryStringEquals(engine->descriptors[index].id, descriptor->id))
        {
            return NX_DISCOVERY_STATUS_DUPLICATE_ID;
        }
    }

    engine->descriptors[engine->count] = *descriptor;
    engine->count += 1;

    return NX_DISCOVERY_STATUS_OK;
}

size_t NxDiscoveryEngine_Count(const NxDiscoveryEngine* engine)
{
    if (engine == 0)
    {
        return 0;
    }

    return engine->count;
}

NxDiscoveryStatus NxDiscoveryEngine_Get(
    const NxDiscoveryEngine* engine,
    size_t index,
    const NxExperimentDescriptor** descriptor_out)
{
    if (engine == 0 || descriptor_out == 0 || engine->descriptors == 0)
    {
        return NX_DISCOVERY_STATUS_INVALID_ARGUMENT;
    }

    *descriptor_out = 0;

    if (index >= engine->count)
    {
        return NX_DISCOVERY_STATUS_NOT_FOUND;
    }

    *descriptor_out = &engine->descriptors[index];
    return NX_DISCOVERY_STATUS_OK;
}

NxDiscoveryStatus NxDiscoveryEngine_FindById(
    const NxDiscoveryEngine* engine,
    const char* id,
    const NxExperimentDescriptor** descriptor_out)
{
    size_t index = 0;

    if (engine == 0 || id == 0 || descriptor_out == 0 || engine->descriptors == 0)
    {
        return NX_DISCOVERY_STATUS_INVALID_ARGUMENT;
    }

    *descriptor_out = 0;

    for (index = 0; index < engine->count; ++index)
    {
        if (NxDiscoveryStringEquals(engine->descriptors[index].id, id))
        {
            *descriptor_out = &engine->descriptors[index];
            return NX_DISCOVERY_STATUS_OK;
        }
    }

    return NX_DISCOVERY_STATUS_NOT_FOUND;
}

size_t NxDiscoveryEngine_Discover(
    const NxDiscoveryEngine* engine,
    NxDiscoveryPredicate predicate,
    void* user_data,
    const NxExperimentDescriptor** results,
    size_t result_capacity)
{
    size_t index = 0;
    size_t matched = 0;

    if (engine == 0 || engine->descriptors == 0 || predicate == 0)
    {
        return 0;
    }

    for (index = 0; index < engine->count; ++index)
    {
        const NxExperimentDescriptor* descriptor = &engine->descriptors[index];

        if (predicate(descriptor, user_data) != 0)
        {
            if (results != 0 && matched < result_capacity)
            {
                results[matched] = descriptor;
            }

            matched += 1;
        }
    }

    return matched;
}

int NxDiscoveryPredicate_All(
    const NxExperimentDescriptor* descriptor,
    void* user_data)
{
    (void)user_data;
    return descriptor != 0 ? 1 : 0;
}

int NxDiscoveryPredicate_Ready(
    const NxExperimentDescriptor* descriptor,
    void* user_data)
{
    (void)user_data;

    if (descriptor == 0)
    {
        return 0;
    }

    return descriptor->state == NX_EXPERIMENT_STATE_READY ? 1 : 0;
}

int NxDiscoveryPredicate_Component(
    const NxExperimentDescriptor* descriptor,
    void* user_data)
{
    const char* component = (const char*)user_data;

    if (descriptor == 0 || component == 0)
    {
        return 0;
    }

    return NxDiscoveryStringEquals(descriptor->component, component);
}
