#ifndef NEXIORA_NCP_STRING_NXSTRING_H
#define NEXIORA_NCP_STRING_NXSTRING_H

#include "Nexiora/NCP/Common/NxResult.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NXM-0003 — NxString
 *
 * Nexiora string primitives are designed as low-overhead replacements for
 * common C runtime operations. The first implementation is portable and
 * conservative; future versions will add SIMD-specialized variants selected
 * by the HAL.
 */

typedef struct NxStringView {
    const char* data;
    size_t length;
} NxStringView;

size_t nx_string_length(const char* text);
size_t nx_string_length_bounded(const char* text, size_t max_length);
int nx_string_compare(const char* left, const char* right);
int nx_string_compare_n(const char* left, const char* right, size_t max_length);
NxResult nx_string_copy(char* destination, size_t destination_size, const char* source);
NxResult nx_string_copy_n(char* destination, size_t destination_size, const char* source, size_t source_length);
const char* nx_string_find_char(const char* text, char value);
NxStringView nx_string_view_from_cstr(const char* text);
int nx_string_view_equals(NxStringView left, NxStringView right);

#ifdef __cplusplus
}
#endif

#endif
