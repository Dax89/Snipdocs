#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <cassert>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// NOLINTBEGIN
typedef struct _VectHeader {
    ptrdiff_t length;
    ptrdiff_t capacity;
    ptrdiff_t itemsize;
    char ptr[1];
} _VectHeader;
// NOLINTEND

#define Vect(T) T*

// clang-format off
#define vect_header(self) ((_VectHeader*)(((char*)(self)) - offsetof(_VectHeader, ptr)))
#define vect_create_n(T, n) (Vect(T))_vect_create_n(sizeof(T), n)
#define vect_create(T) vect_create_n(T, 0)
#define vect_begin(self) (self)
#define vect_end(self) ((self) + vect_header(self)->length)
#define vect_first(self) ((self)[0])
#define vect_last(self) ((self)[vect_header(self)->length - 1])
#define vect_resize(T, self, n) self = (Vect(T))_vect_resize(self, n)
#define vect_reserve(T, self, cap) self = (Vect(T))_vect_ensure_capacity(self, cap)
#define vect_clear(self) vect_header(self)->length = 0
#define vect_length(self) (vect_header(self)->length)
#define vect_capacity(self) (vect_header(self)->capacity)
#define vect_empty(self) (!(self) || !vect_header(self)->length)
#define vect_destroy(self) if(self) free(vect_header(self))

#define vect_insert(T, self, idx, ...) do{ \
    self = (Vect(T))_vect_insert(self, idx); \
    self[idx] = __VA_ARGS__; \
} while(0)

#define vect_append(T, self, ...) do{ \
    self = (Vect(T))_vect_insert(self, -1); \
    self[vect_length(self) - 1] = __VA_ARGS__; \
} while(0)

#define vect_pop_n(self, n) do { \
    _VectHeader* hdr = vect_header(self); \
    if(hdr->length > (n)) hdr->length -= (n); \
    else hdr->length = 0; \
} while(0)

#define vect_pop(self) vect_pop_n(self, 1)

#define vect_foreach(T, item, self) \
    for(T* item = vect_begin(self); item != vect_end(self); item++) // NOLINT

// clang-format on

// NOLINTBEGIN
inline Vect(void) _vect_create_n(ptrdiff_t itemsize, ptrdiff_t n) {
    ptrdiff_t cap = (n ? (n + 1) : sizeof(ptrdiff_t)) << 1;
    _VectHeader* hdr =
        (_VectHeader*)calloc(1, sizeof(_VectHeader) + (itemsize * cap));
    if(!hdr) return nullptr;

    hdr->length = n;
    hdr->capacity = cap;
    hdr->itemsize = itemsize;
    return hdr->ptr;
}

inline Vect(void) _vect_ensure_capacity(Vect(void) self, ptrdiff_t newcap) {
    _VectHeader* hdr = vect_header(self);
    if(hdr->capacity >= newcap) return self;

    _VectHeader* newhdr =
        (_VectHeader*)calloc(1, sizeof(_VectHeader) + (hdr->itemsize * newcap));
    memcpy(newhdr, hdr, sizeof(_VectHeader) + (hdr->itemsize * hdr->length));
    newhdr->capacity = newcap;
    free(hdr);
    return newhdr->ptr;
}

inline Vect(void) _vect_resize(Vect(void) self, ptrdiff_t newn) {
    self = _vect_ensure_capacity(self, newn);
    vect_header(self)->length = newn;
    return self;
}

inline Vect(void) _vect_insert(Vect(void) self, ptrdiff_t idx) {
    _VectHeader* hdr = vect_header(self);
    ptrdiff_t len = vect_header(self)->length;
    if(idx == -1) idx = len; // Append mode
    assert(idx <= len);      // Index out of range

    ptrdiff_t newlen = len + 1;
    self = _vect_ensure_capacity(self, newlen);

    // If we are appending (pos == len), skip memmove
    if(idx < len) {
        // Shift existing content after the position
        // (+1 to include null terminator)
        memmove((char*)self + ((idx + 1) * hdr->itemsize),
                (char*)self + (idx * hdr->itemsize),
                (len - idx) * hdr->itemsize);
    }

    vect_header(self)->length = newlen; // Update the length
    return self;
}

inline void vect_remove(Vect(void) self, ptrdiff_t idx) {
    _VectHeader* hdr = vect_header(self);
    ptrdiff_t len = hdr->length;
    assert(idx >= 0 && idx < len); // Index is out of range

    // Shift elements left if not removing the last element
    if(idx < len - 1) {
        memmove((char*)self + (idx * hdr->itemsize),
                (char*)self + ((idx + 1) * hdr->itemsize),
                (len - idx - 1) * hdr->itemsize);
    }

    hdr->length--; // Update length
}

inline void vect_remove_n(Vect(void) self, ptrdiff_t start, ptrdiff_t n) {
    _VectHeader* hdr = vect_header(self);
    if(start < 0 || start >= hdr->length || n <= 0) return;

    ptrdiff_t end = start + n;
    if(end > hdr->length) end = hdr->length;

    // Shift remaining elements left
    memmove((char*)self + (start * hdr->itemsize),
            (char*)self + (end * hdr->itemsize),
            (hdr->length - end) * hdr->itemsize);

    hdr->length -= (end - start);
}
// NOLINTEND

#if defined(__cplusplus)
}
#endif
