#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Public API
// NOLINTBEGIN
typedef struct _Vect {
    size_t capacity;
    size_t length;
    size_t esize;
    void* data;
} _Vect;
// NOLINTEND

#define Vect(T) _Vect

#define vect_create(T) _vect_create(sizeof(T), 0)
#define vect_create_n(T, n) _vect_create(sizeof(T), n)
#define vect_at(T, self, idx) (T*)_vect_at(self, idx);

#define vect_append(T, self, ...)                                              \
    ((T*)_vect_checkcapacity(self))[(self)->length++] = __VA_ARGS__

#define vect_foreach(T, n, self)                                               \
    if((self)->data)                                                           \
        for(T* n = (T*)((self)->data);                                         \
            (n) < ((T*)((self)->data) + (self)->length); (n)++)

bool vect_isempty(const _Vect* self);
size_t vect_getlength(const _Vect* self);
size_t vect_getcapacity(const _Vect* self);
void vect_clear(_Vect* self);
void vect_destroy(_Vect* self);

// Private API
// NOLINTBEGIN
_Vect _vect_create(size_t esize, size_t cap);
void* _vect_checkcapacity(_Vect* self);
void* _vect_at(const _Vect* self, size_t idx);
// NOLINTEND

#if defined(__cplusplus)
}
#endif
