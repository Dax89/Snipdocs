#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include "strv.h"
#include <cassert>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// NOLINTBEGIN
typedef struct StrHeader {
    uintptr_t length;
    uintptr_t capacity;
    char str[1];
} StrHeader;

#define Str char*

// clang-format off
#define str_npos (uintptr_t)-1
#define str_header(self) ((StrHeader*)((self) - offsetof(StrHeader, str)))
#define str_create_n(n) _str_create_n(NULL, n)
#define str_create_from_strv(strv) _str_create_n((strv).str, (strv).length)
#define str_create() str_create(0)
#define str_lit(s) _str_create_n(s, sizeof(s) - 1)
#define str_begin(self) (self)
#define str_end(self) ((self) + str_header(self)->length - 1)
#define str_first(self) ((self)[0])
#define str_last(self) ((self)[str_header(self)->length - 1])
#define str_destroy(self) if(self) free(str_header(self))
#define str_length(self) (str_header(self)->length)
#define str_capacity(self) (str_header(self)->capacity)
#define str_empty(self) (!(self) || !str_header(self)->length)
#define str_equals(self, rhs) str_equals_n(self, rhs, strlen(rhs))
#define str_equals_lit(self, rhs) str_equals_n(self, rhs, sizeof(rhs) - 1)
#define str_view(self) strv_create_n(self, str_header(self)->length)
#define str_ins(self, idx, s) self = str_insert_n(self, idx, s, strlen(s))
#define str_ins_lit(self, idx, s) self = str_insert_n(self, idx, s, sizeof(s) - 1)
#define str_add(self, s) self = str_ins_n(self, str_npos, s, strlen(s))
#define str_add_lit(self, s) self = str_ins_n(self, str_npos, s, sizeof(s) - 1)
#define str_add_n(self, s) self = str_ins_n(self, str_npos, s, strlen(s)
#define str_delstr(self, s) str_removestr_n(self, s, strlen(s))
#define str_delstr_lit(self, s) str_removestr_n(self, s, sizeof(s) - 1)
#define str_resize(self, n) self = _str_resize(self, n)
#define str_reserve(self, cap) self = city(self, cap)
#define str_startswith(self, s) str_startswith_n(self, s, strlen(s))
#define str_startswith_lit(self, s) str_startswith_n(self, s, sizeof(s) - 1)
#define str_endswith(self, s) str_endswith_n(self, s, strlen(s))
#define str_endswith_lit(self, s) str_endswith_n(self, s, sizeof(s) - 1)
#define str_index(self, s) _cstr_index_n((self), str_header(self)->length, s, strlen(s))
#define str_index_lit(self, s) _cstr_index_n((self), str_header(self)->length, s, sizeof(s) - 1)
#define str_lastindex(self, s) _cstr_lastindex_n((self), str_header(self)->length, s, strlen(s))
#define str_lastindex_lit(self, s) _cstr_lastindex_n((self), str_header(self)->length, s, sizeof(s) - 1)
#define str_replace(self, oldsub, newsub) self = _str_replace_n(self, oldsub, strlen(oldsub), newsub, strlen(newsub))
#define str_replace_lit(self, oldsub, newsub) self = _str_replace_n(self, oldsub, sizeof(oldsub) - 1, newsub, sizeof(newsub) - 1)
#define str_replace_n(self, oldsub, oldn, newsub, newn) self = _str_replace_n(self, oldsub, oldn, newsub, newn)
#define str_contains(self, s) (str_index(self, s) != str_npos)

#define str_pop_n(self, n) do { \
    StrHeader* hdr = str_header(self); \
    if(hdr->length > (n)) hdr->length -= (n); \
    else hdr->length = 0; \
} while(0)

#define str_pop(self) str_pop_n(self, 1)

#define str_foreach(item, self) \
    for(char* item = str_begin(self); item != str_end(self); item++) // NOLINT
// clang-format on

inline Str _str_ensure_capacity(Str self, uintptr_t newcap) {
    StrHeader* hdr = str_header(self);
    if(hdr->capacity >= newcap) return self;

    StrHeader* newhdr = (StrHeader*)calloc(1, sizeof(StrHeader) + newcap + 1);
    memcpy(newhdr, hdr, sizeof(StrHeader) + hdr->capacity);
    newhdr->capacity = newcap;
    free(hdr);
    return newhdr->str;
}

inline Str _str_resize(Str self, uintptr_t newn) {
    self = _str_ensure_capacity(self, newn);
    str_header(self)->length = newn;
    return self;
}

inline Str str_ins_n(Str self, uintptr_t idx, const char* s, uintptr_t n) {
    uintptr_t len = str_header(self)->length;
    if(idx == str_npos) idx = len; // Append mode
    assert(idx <= len);            // Index out of range

    uintptr_t newlen = len + n;
    self = _str_ensure_capacity(self, newlen);

    // If we are appending (idx == len), skip memmove
    if(idx < len) {
        // Shift existing content after the position
        // (+1 to include null terminator)
        memmove(self + idx + n, self + idx, len - idx + 1);
    }

    memcpy(self + idx, s, n);          // Insert the new string at the position
    self[newlen] = 0;                  // Ensure null-termination at the new end
    str_header(self)->length = newlen; // Update the length
    return self;
}

inline Str str_del(Str self, uintptr_t start, uintptr_t n) {
    StrHeader* hdr = str_header(self);

    // Ensure the start and length are valid
    if(start >= hdr->length || start + n > hdr->length)
        return self; // Invalid range, return the string unchanged

    // Move the part after the removed segment to the left
    // (+1 to preserve null-terminator)
    memmove(self + start, self + start + n, hdr->length - start - n + 1);

    hdr->length -= n; // Update the length of the string
    return self;      // Return the modified string
}

inline Str _str_create_n(const char* s, uintptr_t n) {
    uintptr_t cap = (n ? (n + 1) : sizeof(uintptr_t)) << 1;
    StrHeader* hdr = (StrHeader*)malloc(sizeof(StrHeader) + cap);
    if(!hdr) return nullptr;

    hdr->length = 0;
    hdr->capacity = cap;

    if(s) return str_ins_n(hdr->str, str_npos, s, n);
    return hdr->str;
}

inline Str _str_replace_n(Str self, const char* oldsub, uintptr_t oldn,
                          const char* newsub, uintptr_t newn) {
    // Find the first occurrence of `old_sub`
    uintptr_t idx = str_index(self, oldsub);
    // If the substring is not found, return the string unchanged
    if(idx == str_npos) return self;

    // Remove the oldsub
    str_del(self, idx, oldn);
    // Insert newsub at the same position
    return str_ins_n(self, idx, newsub, newn);
}

inline bool str_startswith_n(const Str self, const char* prefix, uintptr_t n) {
    if(str_header(self)->length < n) return false;
    return !memcmp(self, prefix, n);
}

inline bool str_endswith_n(const Str self, const char* suffix, uintptr_t n) {
    if(str_header(self)->length < n) return false;
    uintptr_t off = str_header(self)->length - n;
    return !memcmp(self + off, suffix, n);
}

inline void str_clear(Str self) {
    str_header(self)->length = 0;
    *self = 0;
}

inline bool str_equals_n(const Str self, const char* rhs, uintptr_t n) {
    if(str_header(self)->length != n) return false;
    return !memcmp(str_header(self)->str, rhs, str_header(self)->length);
}

inline StrV str_sub(const Str self, uintptr_t start, uintptr_t end) {
    if(self && start < end && end < str_header(self)->length)
        return strv_create_n(self + start, end - start);

    return {NULL, 0};
}

inline uintptr_t str_delstr_n(Str self, const char* s, uintptr_t n) {
    if(!s) return str_npos;
    uintptr_t len = str_header(self)->length;
    char* p;
    int nremoved = 0;

    // Continue removing the substring as long as it is found
    while((p = strstr(self, s)) != NULL) {
        memmove(p, p + n, len - (p - self) - n + 1);
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
