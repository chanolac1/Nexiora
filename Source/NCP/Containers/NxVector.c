#include "Nexiora/NCP/Containers/NxVector.h"
#include "Nexiora/NCP/Memory/NxMemory.h"

#include <stdint.h>
#include <string.h>

static size_t nx_vector_next_capacity(
    size_t current,
    size_t required,
    NxVectorGrowthPolicy policy
)
{
    if (current == 0) {
        current = 8;
    }

    size_t next = current;

    while (next < required) {
        size_t previous = next;

        switch (policy) {
            case NX_VECTOR_GROWTH_PERFORMANCE:
                next = next * 2;
                break;

            case NX_VECTOR_GROWTH_MEMORY:
                next = next + (next / 4) + 1;
                break;

            case NX_VECTOR_GROWTH_BALANCED:
            default:
                next = next + (next / 2) + 1;
                break;
        }

        if (next <= previous) {
            return required;
        }
    }

    return next;
}

NxResult nx_vector_create(
    NxVector* vector,
    size_t element_size,
    size_t initial_capacity,
    NxVectorGrowthPolicy policy
)
{
    if (vector == NULL || element_size == 0) {
        return NX_ERROR_ARGUMENT;
    }

    vector->data = NULL;
    vector->element_size = element_size;
    vector->size = 0;
    vector->capacity = 0;
    vector->growth_policy = policy;

    if (initial_capacity > 0) {
        return nx_vector_reserve(vector, initial_capacity);
    }

    return NX_OK;
}

void nx_vector_destroy(NxVector* vector)
{
    if (vector == NULL) {
        return;
    }

    if (vector->data != NULL) {
        nx_memory_free(vector->data);
    }

    vector->data = NULL;
    vector->element_size = 0;
    vector->size = 0;
    vector->capacity = 0;
    vector->growth_policy = NX_VECTOR_GROWTH_BALANCED;
}

NxResult nx_vector_reserve(NxVector* vector, size_t new_capacity)
{
    if (vector == NULL || vector->element_size == 0) {
        return NX_ERROR_ARGUMENT;
    }

    if (new_capacity <= vector->capacity) {
        return NX_OK;
    }

    if (new_capacity > (SIZE_MAX / vector->element_size)) {
        return NX_ERROR_ARGUMENT;
    }

    const size_t bytes = new_capacity * vector->element_size;
    void* new_data = nx_memory_allocate_aligned(bytes, NX_MEMORY_DEFAULT_ALIGNMENT);

    if (new_data == NULL) {
        return NX_ERROR_MEMORY;
    }

    if (vector->data != NULL && vector->size > 0) {
        memcpy(new_data, vector->data, vector->size * vector->element_size);
        nx_memory_free(vector->data);
    }

    vector->data = new_data;
    vector->capacity = new_capacity;

    return NX_OK;
}

NxResult nx_vector_push_back(NxVector* vector, const void* element)
{
    if (vector == NULL || element == NULL || vector->element_size == 0) {
        return NX_ERROR_ARGUMENT;
    }

    if (vector->size == vector->capacity) {
        if (vector->size == SIZE_MAX) {
            return NX_ERROR_ARGUMENT;
        }

        const size_t required = vector->size + 1;
        const size_t next = nx_vector_next_capacity(
            vector->capacity,
            required,
            vector->growth_policy
        );

        const NxResult result = nx_vector_reserve(vector, next);
        if (result != NX_OK) {
            return result;
        }
    }

    unsigned char* destination = (unsigned char*)vector->data +
        (vector->size * vector->element_size);

    memcpy(destination, element, vector->element_size);
    vector->size++;

    return NX_OK;
}

NxResult nx_vector_pop_back(NxVector* vector, void* out_element)
{
    if (vector == NULL || vector->size == 0 || vector->element_size == 0) {
        return NX_ERROR_ARGUMENT;
    }

    vector->size--;

    unsigned char* source = (unsigned char*)vector->data +
        (vector->size * vector->element_size);

    if (out_element != NULL) {
        memcpy(out_element, source, vector->element_size);
    }

    return NX_OK;
}

void* nx_vector_at(NxVector* vector, size_t index)
{
    if (vector == NULL || index >= vector->size) {
        return NULL;
    }

    return (unsigned char*)vector->data + (index * vector->element_size);
}

const void* nx_vector_at_const(const NxVector* vector, size_t index)
{
    if (vector == NULL || index >= vector->size) {
        return NULL;
    }

    return (const unsigned char*)vector->data + (index * vector->element_size);
}

void nx_vector_clear(NxVector* vector)
{
    if (vector == NULL) {
        return;
    }

    vector->size = 0;
}

bool nx_vector_is_empty(const NxVector* vector)
{
    return vector == NULL || vector->size == 0;
}

size_t nx_vector_size(const NxVector* vector)
{
    return vector != NULL ? vector->size : 0;
}

size_t nx_vector_capacity(const NxVector* vector)
{
    return vector != NULL ? vector->capacity : 0;
}

void* nx_vector_data(NxVector* vector)
{
    return vector != NULL ? vector->data : NULL;
}

const void* nx_vector_data_const(const NxVector* vector)
{
    return vector != NULL ? vector->data : NULL;
}
