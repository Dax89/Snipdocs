#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "strv.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// NOLINTBEGIN
typedef struct _StrLong {
    size_t capacity;
    char* data;
} _StrLong;
// NOLINTEND

typedef struct Str {
    size_t length;

    union {
        _StrLong l;
        char s[sizeof(_StrLong)];
    };
} Str;

Str str_create(const char* s);
Str str_create_n(const char* s, size_t start, size_t n);
void str_appendstr(Str* self, const Str* s);
void str_append_n(Str* self, const char* s, size_t n);
void str_append(Str* self, const char* s);
bool str_startswithstr(Str* self, const Str* s);
bool str_startswith_n(Str* self, const char* s, size_t n);
bool str_startswith(Str* self, const char* s);
bool str_endswithstr(Str* self, const Str* s);
bool str_endswith_n(Str* self, const char* s, size_t n);
bool str_endswith(Str* self, const char* s);
int str_indexofstr(Str* self, const Str* s);
int str_indexof_n(Str* self, const char* s, size_t n);
int str_indexof(Str* self, const char* s);
int str_lastindexofstr(Str* self, const Str* s);
int str_lastindexof_n(Str* self, const char* s, size_t n);
int str_lastindexof(Str* self, const char* s);
bool str_issmall(const Str* self);
bool str_isempty(const Str* self);
size_t str_getlength(const Str* self);
size_t str_getcapacity(const Str* self);
const char* str_getdata(const Str* self);
bool str_equals(const Str* self, const Str* rhs);
void str_clear(Str* self);
void str_destroy(Str* self);

// Strv interface
inline Strv str_view(const Str* s) {
    return strv_create_n(str_getdata(s), 0, str_getlength(s));
}

inline Strv str_sub(const Str* s, size_t start, size_t n) {
    return strv_create_n(str_getdata(s), start, n);
}

#if defined(__cplusplus)
}
#endif
