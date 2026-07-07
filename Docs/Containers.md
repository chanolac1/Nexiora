# Nexiora Containers

## NxVector

`NxVector` is the first official Nexiora container. It stores fixed-size elements in contiguous memory and uses the Nexiora Memory Manager for allocation.

### Why not use a standard container?

Nexiora is written in C and requires tight control over memory, performance, alignment, statistics and future evolution.

### Example

```c
NxVector v;
nx_vector_create(&v, sizeof(int), 0, NX_VECTOR_GROWTH_BALANCED);

int value = 42;
nx_vector_push_back(&v, &value);

int* item = nx_vector_at(&v, 0);

nx_vector_destroy(&v);
```
