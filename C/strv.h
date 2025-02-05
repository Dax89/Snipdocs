#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

// NOLINTBEGIN
typedef struct StrV {
    const char* str;
    ptrdiff_t length;
} StrV;
// NOLINTEND

#define strv_lit(s) strv_create_n(s, sizeof(s) - 1)
#define strv_create(s) strv_create_n(s, strlen(s))
#define strv_valid(self) ((self).str && (self).length)
#define strv_length(self) ((self).length)
#define strv_ptr(self) ((self).str)
#define strv_empty(self) (!(self).str || !(self).length)
#define strv_equals(self, rhs) strv_equals_n(self, rhs, strlen(rhs))
#define strv_equals_lit(self, rhs) strv_equals_n(self, rhs, sizeof(rhs) - 1)
#define strv_startswith(self, s) strv_startswith_n(self, s, strlen(s))
#define strv_startswith_lit(self, s) strv_startswith_n(self, s, sizeof(s) - 1)
#define strv_endswith(self, s) strv_endswith_n(self, s, strlen(s))
#define strv_endswith_lit(self, s) strv_endswith_n(self, s, sizeof(s) - 1)
#define strv_lastindexof(self, s) _strv_lastindexof(self, s, strlen(s))
#define strv_lastindexof_lit(self, s) _strv_lastindexof(self, s, sizeof(s) - 1)
#define strv_contains(self, s) (strv_indexof(self, s) != -1)

// NOLINTBEGIN
inline StrV strv_create_n(const char* s, ptrdiff_t n) {
    StrV self;
    self.str = s;
    self.length = n;
    return self;
}

inline bool strv_equals_n(StrV self, const char* rhs, ptrdiff_t n) {
    if(self.length != n) return false;
    return !memcmp(self.str, rhs, self.length);
}

inline ptrdiff_t strv_indexof(StrV self, const char* s) {
    const char* pos = strstr(self.str, s);
    return pos ? pos - self.str : -1;
}

inline bool strv_startswith_n(StrV self, const char* prefix, ptrdiff_t n) {
    if(self.length < n) return false;
    return !memcmp(self.str, prefix, n);
}

inline bool strv_endswith_n(StrV self, const char* suffix, ptrdiff_t n) {
    if(self.length < n) return false;
    ptrdiff_t off = self.length - n;
    return !memcmp(self.str + off, suffix, n);
}

inline StrV strv_sub(StrV self, ptrdiff_t start, ptrdiff_t end) {
    StrV res = {NULL, 0};
    if(start < 0) start = self.length + start;
    if(end <= 0) end = self.length + end;

    if(strv_valid(self) && start >= 0 && start < end && end < self.length) {
        res.str = self.str + start;
        res.length = end - start;
    }

    return res;
}

inline ptrdiff_t _strv_lastindexof(StrV self, const char* s, ptrdiff_t n) {
    const char* pos = NULL;

    for(ptrdiff_t i = self.length - n; i >= 0; i--) {
        pos = strstr(self.str + i, s);
        if(pos) return i;
    }

    return -1;
}
// NOLINTEND

#if defined(__cplusplus)
}
#endif
