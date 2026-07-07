# NXM-0003 — NxString

NxString es el primer módulo de primitivas de cadenas de Nexiora Core Platform.

## Archivos

```text
Include/Nexiora/NCP/String/NxString.h
Source/NCP/String/NxString.c
```

## API inicial

```c
size_t nx_string_length(const char* text);
size_t nx_string_length_bounded(const char* text, size_t max_length);
int nx_string_compare(const char* left, const char* right);
int nx_string_compare_n(const char* left, const char* right, size_t max_length);
NxResult nx_string_copy(char* destination, size_t destination_size, const char* source);
NxResult nx_string_copy_n(char* destination, size_t destination_size, const char* source, size_t source_length);
const char* nx_string_find_char(const char* text, char value);
NxStringView nx_string_view_from_cstr(const char* text);
int nx_string_view_equals(NxStringView left, NxStringView right);
```

## Benchmarks agregados

- `nx_string_length 64 bytes`
- `nx_string_compare 64 bytes`
- `nx_string_copy 64 bytes`

## Próximo paso
Agregar dispatch especializado por hardware usando HAL:

- portable baseline
- SSE2
- AVX2
- AVX-512
