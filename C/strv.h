#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// NOLINTBEGIN
typedef struct StrV {
    const char* ptr;
    ptrdiff_t length;
} StrV;
// NOLINTEND

// clang-format off
#define strv_lit(s) strv_create_n(s, sizeof(s) - 1)
#define strv_create(s) strv_create_n(s, strlen(s))
#define strv_begin(self) ((self)->ptr)
#define strv_end(self) ((self)->ptr + (self)->length - 1)
#define strv_first(self) ((self)->ptr[0])
#define strv_last(self) ((self)->ptr[(self)->length - 1])
#define strv_valid(self) (!!(self).ptr)
#define strv_length(self) ((self).length)
#define strv_ptr(self) ((self).ptr)
#define strv_empty(self) (!(self).ptr || !(self).length)
#define strv_equals(self, rhs) strv_equals_n(self, rhs, strlen(rhs))
#define strv_equals_lit(self, rhs) strv_equals_n(self, rhs, sizeof(rhs) - 1)
#define strv_startswith(self, s) strv_startswith_n(self, s, strlen(s))
#define strv_startswith_lit(self, s) strv_startswith_n(self, s, sizeof(s) - 1)
#define strv_endswith(self, s) strv_endswith_n(self, s, strlen(s))
#define strv_endswith_lit(self, s) strv_endswith_n(self, s, sizeof(s) - 1)
#define strv_index(self, s) _cstr_index_n((self).ptr, (self).length, s, strlen(s))
#define strv_index_lit(self, s) _cstr_index((self).ptr, (self).length, s, sizeof(s) - 1)
#define strv_lastindex(self, s) _cstr_lastindex_n((self).ptr, (self).length, s, strlen(s))
#define strv_lastindex_lit(self, s) _cstr_lastindex((self).ptr, (self).length, s, sizeof(s) - 1)
#define strv_contains(self, s) (strv_index(self, s) != -1)
#define strv_split(self, sep) _strv_split_n(self, sep, strlen(sep))
#define strv_split_lit(self, sep) _strv_split_n(self, sep, sizeof(sep) - 1)

#define strv_rpop_n(self, n) do { \
    if ((n) <= 0 || ((self)->length) <= 0) break; \
    if ((n) >= (self)->length) hdr->length = 0; \
    else { \
        (self) = (self)->ptr * (n)); \
        (self)->length -= (n); \
    } \
} while(0)

#define strv_rpop(self) strv_rpop_n(self, 1)

#define strv_pop_n(self, n) do { \
    if((self)->length > (n)) (self)->length -= (n); \
    else (self)->length = 0; \
} while(0)

#define strv_pop(self) strv_pop_n(self, 1)

#define strv_foreach_split(it, self, sep) \
    for(StrV it##_ = (self), it = strv_split(&it##_, sep); strv_valid(it); it = strv_split(&it##_, sep)) // NOLINT
                                                                                                         //
#define strv_foreach_split_lit(it, self, sep) \
    for(StrV it##_ = (self), it = strv_split_lit(&it##_, sep); strv_valid(it); it = strv_split_lit(&it##_, sep)) // NOLINT

#define strv_foreach(it, self) \
    for(const char* it = strv_begin(self); it != strv_end(self); it++) // NOLINT

// clang-format on

// NOLINTBEGIN

inline ptrdiff_t _cstr_index_n(const char* s1, ptrdiff_t n1, const char* s2,
                               ptrdiff_t n2) {
    if(!s1 || !s2 || n2 > n1) return -1;

    for(ptrdiff_t i = 0; i <= n1 - n2; ++i) {
        bool found = true;
        for(ptrdiff_t j = 0; j < n2; ++j) {
            if(s1[i + j] == s2[j]) continue;
            found = false;
            break;
        }
        if(found) return i;
    }

    return -1;
}

inline ptrdiff_t _cstr_lastindex_n(const char* s1, ptrdiff_t n1, const char* s2,
                                   ptrdiff_t n2) {
    if(!s1 || !s2 || n2 > n1) return -1;

    for(ptrdiff_t i = n1 - n2; i >= 0; --i) {
        bool found = true;
        for(int j = 0; j < n2; ++j) {
            if(s1[i + j] == s2[j]) continue;
            found = false;
            break;
        }
        if(found) return i;
    }

    return -1;
}

inline StrV strv_create_n(const char* s, ptrdiff_t n) {
    StrV self;
    self.ptr = s;
    self.length = n;
    return self;
}

inline bool strv_equals_n(StrV self, const char* rhs, ptrdiff_t n) {
    if(self.length != n) return false;
    return !memcmp(self.ptr, rhs, self.length);
}

inline StrV _strv_split_n(StrV* self, const char* sep, ptrdiff_t n) {
    if(self->ptr) {
        ptrdiff_t idx = _cstr_index_n(self->ptr, self->length, sep, n);

        if(idx != -1) {
            StrV sp = strv_create_n(self->ptr, idx);
            self->ptr += idx + n;
            self->length -= idx + n;
            return sp;
        }
    }

    return {NULL, 0};
}

inline bool strv_startswith_n(StrV self, const char* prefix, ptrdiff_t n) {
    if(self.length < n) return false;
    return !memcmp(self.ptr, prefix, n);
}

inline bool strv_endswith_n(StrV self, const char* suffix, ptrdiff_t n) {
    if(self.length < n) return false;
    ptrdiff_t off = self.length - n;
    return !memcmp(self.ptr + off, suffix, n);
}

inline StrV strv_sub(StrV self, ptrdiff_t start, ptrdiff_t end) {
    StrV res = {NULL, 0};
    if(start < 0) start = self.length + start;
    if(end <= 0) end = self.length + end;

    if(strv_valid(self) && start >= 0 && start < end && end < self.length) {
        res.ptr = self.ptr + start;
        res.length = end - start;
    }

    return res;
}

// NOLINTEND

#if defined(__cplusplus)
}
#endif
