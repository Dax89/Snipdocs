#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "strv.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// NOLINTBEGIN
typedef struct _StrHeader {
    ptrdiff_t length;
    ptrdiff_t capacity;
    char ptr[1];
} _StrHeader;
// NOLINTEND

#define Str char*

// clang-format off
#define str_header(self) ((_StrHeader*)((self) - offsetof(_StrHeader, ptr)))
#define str_create_n(n) _str_create_n(NULL, n)
#define str_lit(s) _str_create_n(s, sizeof(s) - 1)
#define str_create() str_create(0)
#define str_length(self) (str_header(self)->length)
#define str_capacity(self) (str_header(self)->capacity)
#define str_empty(self) (!(self) || !str_header(self)->length)
#define str_equals(self, rhs) str_equals_n(self, rhs, strlen(rhs))
#define str_equals_lit(self, rhs) str_equals_n(self, rhs, sizeof(rhs) - 1)
#define str_destroy(self) free((self) ? str_header(self) : NULL)
#define str_view(self) strv_create_n(self, str_header(self)->length)
#define str_insert(self, pos, s) self = str_insert_n(self, pos, s, strlen(s))
#define str_insert_lit(self, pos, s) self = str_insert_n(self, pos, s, sizeof(s) - 1)
#define str_append(self, s) self = str_insert_n(self, -1, s, strlen(s))
#define str_append_lit(self, s) self = str_insert_n(self, -1, s, sizeof(s) - 1)
#define str_append_n(self, s) self = str_insert_n(self, -1, s, strlen(s)
#define str_removestr(self, s) str_removestr_n(self, s, strlen(s))
#define str_removestr_lit(self, s) str_removestr_n(self, s, sizeof(s) - 1)
#define str_reserve(self, cap) self = _str_ensure_capacity(self, cap)
#define str_startswith(self, s) str_startswith_n(self, s, strlen(s))
#define str_startswith_lit(self, s) str_startswith_n(self, s, sizeof(s) - 1)
#define str_endswith(self, s) str_endswith_n(self, s, strlen(s))
#define str_endswith_lit(self, s) str_endswith_n(self, s, sizeof(s) - 1)
#define str_lastindexof(self, s) _str_lastindexof(self, s, strlen(s))
#define str_lastindexof_lit(self, s) _str_lastindexof(self, s, sizeof(s) - 1)
#define str_replace(self, oldsub, newsub) self = _str_replace_n(self, oldsub, strlen(oldsub), newsub, strlen(newsub))
#define str_replace_lit(self, oldsub, newsub) self = _str_replace_n(self, oldsub, sizeof(oldsub) - 1, newsub, sizeof(newsub) - 1)
#define str_replace_n(self, oldsub, oldn, newsub, newn) self = _str_replace_n(self, oldsub, oldn, newsub, newn)
#define str_contains(self, s) (str_indexof(self, s) != -1)
// clang-format on

// NOLINTBEGIN
inline Str _str_ensure_capacity(Str self, ptrdiff_t newcap) {
    _StrHeader* hdr = str_header(self);
    if(hdr->capacity >= newcap) return self;

    _StrHeader* newhdr = (_StrHeader*)malloc(sizeof(_StrHeader) + newcap + 1);
    memcpy(newhdr, hdr, sizeof(_StrHeader) + hdr->capacity);
    newhdr->capacity = newcap;
    free(hdr);
    return newhdr->ptr;
}

inline ptrdiff_t str_indexof(Str self, const char* s) {
    const char* pos = strstr(self, s);
    return pos ? pos - self : -1;
}

inline Str str_insert_n(Str self, ptrdiff_t pos, const char* s, ptrdiff_t n) {
    ptrdiff_t len = str_header(self)->length;
    if(pos == -1) pos = len;           // Append mode
    else if(pos > len) return nullptr; // Position out of range

    ptrdiff_t newlen = len + n;
    self = _str_ensure_capacity(self, newlen);

    // If we are appending (pos == len), skip memmove
    if(pos < len) {
        // Shift existing content after the position
        // (+1 to include null terminator)
        memmove(self + pos + n, self + pos, len - pos + 1);
    }

    memcpy(self + pos, s, n);          // Insert the new string at the position
    self[newlen] = 0;                  // Ensure null-termination at the new end
    str_header(self)->length = newlen; // Update the length
    return self;
}

inline Str str_remove(Str self, ptrdiff_t start, ptrdiff_t n) {
    _StrHeader* hdr = str_header(self);

    // Ensure the start and length are valid
    if(start < 0 || start >= hdr->length || n < 0 || start + n > hdr->length)
        return self; // Invalid range, return the string unchanged

    // Move the part after the removed segment to the left
    // (+1 to preserve null-terminator)
    memmove(self + start, self + start + n, hdr->length - start - n + 1);

    hdr->length -= n; // Update the length of the string
    return self;      // Return the modified string
}

inline Str _str_create_n(const char* s, ptrdiff_t n) {
    ptrdiff_t cap = n ? (n + 1) << 1 : 1024;
    _StrHeader* hdr = (_StrHeader*)malloc(sizeof(_StrHeader) + cap);
    if(!hdr) return nullptr;

    hdr->length = 0;
    hdr->capacity = cap;

    if(s) return str_insert_n(hdr->ptr, -1, s, n);
    return hdr->ptr;
}

inline ptrdiff_t _str_lastindexof(Str self, const char* s, ptrdiff_t n) {
    const char* pos = NULL;

    for(ptrdiff_t i = str_header(self)->length - n; i >= 0; i--) {
        pos = strstr(self + i, s);
        if(pos) return i;
    }

    return -1;
}

inline Str _str_replace_n(Str self, const char* oldsub, ptrdiff_t oldn,
                          const char* newsub, ptrdiff_t newn) {
    // Find the first occurrence of `old_sub`
    ptrdiff_t pos = str_indexof(self, oldsub);
    // If the substring is not found, return the string unchanged
    if(pos == -1) return self;
    str_remove(self, pos, oldn);          // Remove the oldsub
    return str_insert(self, pos, newsub); // Insert newsub at the same position
}

inline bool str_startswith_n(const Str self, const char* prefix, ptrdiff_t n) {
    if(str_header(self)->length < n) return false;
    return !memcmp(self, prefix, n);
}

inline bool str_endswith_n(const Str self, const char* suffix, ptrdiff_t n) {
    if(str_header(self)->length < n) return false;
    ptrdiff_t off = str_header(self)->length - n;
    return !memcmp(self + off, suffix, n);
}

inline void str_clear(Str self) {
    str_header(self)->length = 0;
    *self = 0;
}

inline bool str_equals_n(const Str self, const char* rhs, ptrdiff_t n) {
    if(str_header(self)->length != n) return false;
    return !memcmp(str_header(self)->ptr, rhs, str_header(self)->length);
}

inline StrV str_sub(const Str self, ptrdiff_t start, ptrdiff_t end) {
    if(start < 0) start = str_header(self)->length + start;
    if(end <= 0) end = str_header(self)->length + end;

    if(self && start >= 0 && start < end && end < str_header(self)->length)
        return strv_create_n(self + start, end - start);

    return {NULL, 0};
}

inline ptrdiff_t str_removestr_n(Str self, const char* s, ptrdiff_t n) {
    if(!s) return -1;
    ptrdiff_t len = str_header(self)->length;
    char* pos;
    int nremoved = 0;

    // Continue removing the substring as long as it is found
    while((pos = strstr(self, s)) != NULL) {
        memmove(pos, pos + n, len - (pos - self) - n + 1);
        str_header(self)->length -= n;
        nremoved++;
        len = str_header(self)->length;
    }

    return nremoved;
}
// NOLINTEND

#if defined(__cplusplus)
}
#endif
