#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

typedef struct Strv {
    const char* data;
    size_t length;
} Strv;

Strv strv_create(const char* s);
Strv strv_create_n(const char* s, size_t start, size_t n);
bool strv_isempty(const Strv* self);
size_t strv_getlength(const Strv* self);
const char* strv_getdata(const Strv* self);
bool strv_startswith_str(Strv* self, const Strv* s);
bool strv_startswith_n(Strv* self, const char* s, size_t n);
bool strv_startswith(Strv* self, const char* s);
bool strv_endswith_str(Strv* self, const Strv* s);
bool strv_endswith_n(Strv* self, const char* s, size_t n);
bool strv_endswith(Strv* self, const char* s);
int strv_indexof_str(Strv* self, const Strv* s);
int strv_indexof_n(Strv* self, const char* s, size_t n);
int strv_indexof(Strv* self, const char* s);
int strv_lastindexof_str(Strv* self, const Strv* s);
int strv_lastindexof_n(Strv* self, const char* s, size_t n);
int strv_lastindexof(Strv* self, const char* s);
bool strv_equals(const Strv* self, const Strv* rhs);

// GENERIC
// NOLINTBEGIN
int _cstr_indexof(const char* s1, size_t sz1, const char* s2, size_t sz2);
int _cstr_lastindexof(const char* s1, size_t sz1, const char* s2, size_t sz2);

bool _cstr_startswith(const char* s1, size_t sz1, const char* s2, size_t sz2);
bool _cstr_endswith(const char* s1, size_t sz1, const char* s2, size_t sz2);
// NOLINTEND

#if defined(__cplusplus)
}
#endif
