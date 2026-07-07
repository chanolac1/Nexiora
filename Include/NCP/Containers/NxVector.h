#ifndef NEXIORA_NCP_CONTAINERS_NXVECTOR_H
#define NEXIORA_NCP_CONTAINERS_NXVECTOR_H

#include <stddef.h>
#include <stdbool.h>
#include "NCP/Common/NxResult.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxVectorGrowthPolicy {
    NX_VECTOR_GROWTH_BALANCED = 0,
    NX_VECTOR_GROWTH_PERFORMANCE = 1,
    NX_VECTOR_GROWTH_MEMORY = 2
} NxVectorGrowthPolicy;

typedef struct NxVector {
    void* data;
    size_t element_size;
    size_t size;
    size_t capacity;
    NxVectorGrowthPolicy growth_policy;
} NxVector;

NxResult nx_vector_create(NxVector* vector, size_t element_size, size_t initial_capacity, NxVectorGrowthPolicy policy);
void nx_vector_destroy(NxVector* vector);
NxResult nx_vector_reserve(NxVector* vector, size_t new_capacity);
NxResult nx_vector_push_back(NxVector* vector, const void* element);
NxResult nx_vector_pop_back(NxVector* vector, void* out_element);
void* nx_vector_at(NxVector* vector, size_t index);
const void* nx_vector_at_const(const NxVector* vector, size_t index);
void nx_vector_clear(NxVector* vector);
bool nx_vector_is_empty(const NxVector* vector);
size_t nx_vector_size(const NxVector* vector);
size_t nx_vector_capacity(const NxVector* vector);
void* nx_vector_data(NxVector* vector);
const void* nx_vector_data_const(const NxVector* vector);

#ifdef __cplusplus
}
#endif

#endif
