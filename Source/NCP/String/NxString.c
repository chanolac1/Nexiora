#include "Nexiora/NCP/String/NxString.h"

size_t nx_string_length(const char* text) {
    if (text == 0) {
        return 0;
    }

    const char* p = text;
    while (*p != '\0') {
        ++p;
    }
    return (size_t)(p - text);
}

size_t nx_string_length_bounded(const char* text, size_t max_length) {
    if (text == 0) {
        return 0;
    }

    size_t i = 0;
    while (i < max_length && text[i] != '\0') {
        ++i;
    }
    return i;
}

int nx_string_compare(const char* left, const char* right) {
    if (left == right) {
        return 0;
    }
    if (left == 0) {
        return -1;
    }
    if (right == 0) {
        return 1;
    }

    while (*left != '\0' && *left == *right) {
        ++left;
        ++right;
    }

    return ((unsigned char)*left > (unsigned char)*right) -
           ((unsigned char)*left < (unsigned char)*right);
}

int nx_string_compare_n(const char* left, const char* right, size_t max_length) {
    if (max_length == 0 || left == right) {
        return 0;
    }
    if (left == 0) {
        return -1;
    }
    if (right == 0) {
        return 1;
    }

    for (size_t i = 0; i < max_length; ++i) {
        unsigned char l = (unsigned char)left[i];
        unsigned char r = (unsigned char)right[i];
        if (l != r) {
            return (l > r) - (l < r);
        }
        if (l == '\0') {
            return 0;
        }
    }
    return 0;
}

NxResult nx_string_copy(char* destination, size_t destination_size, const char* source) {
    if (destination == 0 || destination_size == 0 || source == 0) {
        return NX_ERROR_ARGUMENT;
    }

    size_t i = 0;
    while (i + 1 < destination_size && source[i] != '\0') {
        destination[i] = source[i];
        ++i;
    }

    destination[i] = '\0';
    return source[i] == '\0' ? NX_OK : NX_ERROR_ARGUMENT;
}

NxResult nx_string_copy_n(char* destination, size_t destination_size, const char* source, size_t source_length) {
    if (destination == 0 || destination_size == 0 || source == 0) {
        return NX_ERROR_ARGUMENT;
    }

    if (destination_size <= source_length) {
        if (destination_size > 0) {
            destination[0] = '\0';
        }
        return NX_ERROR_ARGUMENT;
    }

    for (size_t i = 0; i < source_length; ++i) {
        destination[i] = source[i];
    }
    destination[source_length] = '\0';
    return NX_OK;
}

const char* nx_string_find_char(const char* text, char value) {
    if (text == 0) {
        return 0;
    }

    while (*text != '\0') {
        if (*text == value) {
            return text;
        }
        ++text;
    }

    return value == '\0' ? text : 0;
}

NxStringView nx_string_view_from_cstr(const char* text) {
    NxStringView view;
    view.data = text;
    view.length = nx_string_length(text);
    return view;
}

int nx_string_view_equals(NxStringView left, NxStringView right) {
    if (left.length != right.length) {
        return 0;
    }
    if (left.data == right.data) {
        return 1;
    }
    if (left.data == 0 || right.data == 0) {
        return 0;
    }

    for (size_t i = 0; i < left.length; ++i) {
        if (left.data[i] != right.data[i]) {
            return 0;
        }
    }
    return 1;
}
