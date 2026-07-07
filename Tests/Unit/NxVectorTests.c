#include "Nexiora/NCP/Containers/NxVector.h"

#include <stdio.h>

static int test_vector_push_and_read(void)
{
    NxVector vector;

    if (nx_vector_create(&vector, sizeof(int), 0, NX_VECTOR_GROWTH_BALANCED) != NX_OK) {
        return 1;
    }

    for (int i = 0; i < 1000; ++i) {
        if (nx_vector_push_back(&vector, &i) != NX_OK) {
            nx_vector_destroy(&vector);
            return 2;
        }
    }

    if (nx_vector_size(&vector) != 1000) {
        nx_vector_destroy(&vector);
        return 3;
    }

    for (int i = 0; i < 1000; ++i) {
        int* value = (int*)nx_vector_at(&vector, (size_t)i);
        if (value == NULL || *value != i) {
            nx_vector_destroy(&vector);
            return 4;
        }
    }

    nx_vector_destroy(&vector);
    return 0;
}

static int test_vector_pop(void)
{
    NxVector vector;

    if (nx_vector_create(&vector, sizeof(int), 4, NX_VECTOR_GROWTH_PERFORMANCE) != NX_OK) {
        return 1;
    }

    int first = 10;
    int second = 20;
    int output = 0;

    if (nx_vector_push_back(&vector, &first) != NX_OK) {
        nx_vector_destroy(&vector);
        return 2;
    }

    if (nx_vector_push_back(&vector, &second) != NX_OK) {
        nx_vector_destroy(&vector);
        return 3;
    }

    if (nx_vector_pop_back(&vector, &output) != NX_OK) {
        nx_vector_destroy(&vector);
        return 4;
    }

    if (output != 20) {
        nx_vector_destroy(&vector);
        return 5;
    }

    if (nx_vector_size(&vector) != 1) {
        nx_vector_destroy(&vector);
        return 6;
    }

    nx_vector_destroy(&vector);
    return 0;
}

int main(void)
{
    const int result_push = test_vector_push_and_read();
    const int result_pop = test_vector_pop();

    if (result_push != 0 || result_pop != 0) {
        printf("NxVectorTests FAILED push=%d pop=%d\n", result_push, result_pop);
        return 1;
    }

    printf("NxVectorTests PASSED\n");
    return 0;
}
