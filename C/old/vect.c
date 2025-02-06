#include "vect.h"
#include <stdlib.h>
#include <string.h>

#define vect_DEFAULT_CAPACITY 1024

void* _vect_checkcapacity(_Vect* self) { // NOLINT
    if(self->data && (self->length + 1 < self->capacity)) return self->data;

    size_t oldcap = self->capacity;
    void* olddata = self->data;
    if(!self->capacity) self->capacity = vect_DEFAULT_CAPACITY;
    else self->capacity <<= 1;

    if(self->data) {
        self->data = memcpy(calloc(self->capacity, self->esize), self->data,
                            oldcap * self->esize);

        free(olddata);
    }
    else self->data = calloc(self->capacity, self->esize);

    return self->data;
}

_Vect _vect_create(size_t esize, size_t cap) {
    _Vect self = {cap, 0, esize, NULL};
    if(cap) _vect_checkcapacity(&self);
    return self;
}

void* _vect_at(const _Vect* self, size_t idx) {
    if(!self || !self->data || idx >= self->length) return NULL;
    return (uint8_t*)self->data + (idx * self->esize);
}

bool vect_isempty(const _Vect* self) {
    return self && (!self->data || !self->length || !self->capacity);
}

size_t vect_getlength(const _Vect* self) {
    return self && self->data ? self->length : 0;
}

size_t vect_getcapacity(const _Vect* self) {
    return self && self->data ? self->capacity : 0;
}

void vect_clear(_Vect* self) {
    if(self) self->length = 0;
}

void vect_destroy(_Vect* self) {
    if(!self) return;
    free(self->data);
    self->capacity = self->length = 0;
    self->data = NULL;
}
