#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <cassert>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// NOLINTBEGIN
typedef void (*VectItemDel)(void*);

typedef struct VectHeader {
    uintptr_t length;
    uintptr_t capacity;
    uintptr_t itemsize;
    VectItemDel itemdel;
    char items[1];
} VectHeader;

#define Vect(T) T*

// clang-format off
#define vect_header(self) ((VectHeader*)(((char*)(self)) - offsetof(VectHeader, items)))
#define vect_create_n(T, n) (Vect(T))_vect_create_n(sizeof(T), n)
#define vect_create(T) vect_create_n(T, 0)
#define vect_begin(self) (self)
#define vect_end(self) ((self) + vect_header(self)->length)
#define vect_first(self) ((self)[0])
#define vect_last(self) ((self)[vect_header(self)->length - 1])
#define vect_resize(T, self, n) self = (Vect(T))_vect_resize(self, n))
#define vect_reserve(T, self, cap) self = (Vect(T))_vect_ensure_capacity(self, cap)
#define vect_length(self) (vect_header(self)->length)
#define vect_capacity(self) (vect_header(self)->capacity)
#define vect_empty(self) (!(self) || !vect_header(self)->length)
#define vect_setitemdel(self, itemdelfn) vect_header(self)->itemdel = itemdelfn

#define vect_ins(T, self, idx, ...) do{ \
    self = (Vect(T))_vect_ins(self, idx); \
    self[idx] = __VA_ARGS__; \
} while(0)

#define vect_add(T, self, ...) do{ \
    self = (Vect(T))_vect_ins(self, (uintptr_t)-1); \
    self[vect_length(self) - 1] = __VA_ARGS__; \
} while(0)

#define vect_pop_n(self, n) do { \
    VectHeader* hdr = vect_header(self); \
    if(hdr->length > (n)) hdr->length -= (n); \
    else hdr->length = 0; \
} while(0)

#define vect_pop(self) vect_pop_n(self, 1)

#define vect_foreach(T, item, self) \
    for(T* item = vect_begin(self); item != vect_end(self); item++) // NOLINT

// clang-format on

inline Vect(void) _vect_create_n(uintptr_t itemsize, uintptr_t n) {
    uintptr_t cap = (n ? (n + 1) : sizeof(uintptr_t)) << 1;
    VectHeader* hdr =
        (VectHeader*)calloc(1, sizeof(VectHeader) + (itemsize * cap));
    if(!hdr) return nullptr;
    hdr->length = n;
    hdr->capacity = cap;
    hdr->itemsize = itemsize;
    return hdr->items;
}

inline void vect_clear(Vect(void) self) {
    VectHeader* hdr = vect_header(self);

    if(hdr->itemdel) {
        char* begin = (char*)self;
        char* end = (char*)self + (hdr->length * hdr->itemsize);

        for(char* it = begin; it != end; it += hdr->itemsize)
            hdr->itemdel(it);
    }

    vect_header(self)->length = 0;
}

inline void vect_destroy(Vect(void) self) {
    if(self) {
        vect_clear(self);
        free(vect_header(self));
    }
}

inline Vect(void) _vect_ensure_capacity(Vect(void) self, uintptr_t newcap) {
    VectHeader* hdr = vect_header(self);
    if(hdr->capacity >= newcap) return self;

    VectHeader* newhdr =
        (VectHeader*)calloc(1, sizeof(VectHeader) + (hdr->itemsize * newcap));
    memcpy(newhdr, hdr, sizeof(VectHeader) + (hdr->itemsize * hdr->length));

    newhdr->capacity = newcap;
    newhdr->length = hdr->length;
    newhdr->itemsize = hdr->itemsize;
    newhdr->itemdel = hdr->itemdel;
    free(hdr);
    return newhdr->items;
}

inline Vect(void) _vect_resize(Vect(void) self, uintptr_t newn) {
    self = _vect_ensure_capacity(self, newn);
    vect_header(self)->length = newn;
    return self;
}

inline Vect(void) _vect_ins(Vect(void) self, uintptr_t idx) {
    VectHeader* hdr = vect_header(self);
    uintptr_t len = hdr->length;
    if(idx == (uintptr_t)-1) idx = len; // Append mode
    assert(idx <= len && "Index out of range in _vect_ins()");

    uintptr_t newlen = len + 1;
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

inline void vect_del(Vect(void) self, uintptr_t idx) {
    VectHeader* hdr = vect_header(self);
    uintptr_t len = hdr->length;
    assert(idx < len); // Index is out of range

    if(hdr->itemdel) hdr->itemdel((char*)self + (idx * hdr->itemsize));

    // Shift elements left if not removing the last element
    if(idx < len - 1) {
        memmove((char*)self + (idx * hdr->itemsize),
                (char*)self + ((idx + 1) * hdr->itemsize),
                (len - idx - 1) * hdr->itemsize);
    }

    hdr->length--; // Update length
}

inline void vect_del_n(Vect(void) self, uintptr_t start, uintptr_t n) {
    VectHeader* hdr = vect_header(self);
    if(start >= hdr->length || n <= 0) return;

    uintptr_t end = start + n;
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
