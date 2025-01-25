#include "str.h"
#include <stdlib.h>
#include <string.h>

#define STR_LONGBIT ((size_t)1 << (sizeof(size_t) * 8 - 1))
#define STR_LENMASK ~((size_t)1 << (sizeof(size_t) * 8 - 1))
#define STR_CHECKSMALL(self, len) (len + 1 < sizeof((self)->s))

// NOLINTBEGIN
void _str_checkcapacity(Str* self, size_t n) {
    size_t len = str_getlength(self) + n;

    if(str_issmall(self) && !STR_CHECKSMALL(self, len)) {
        // Small to Long String
        size_t cap = (self->length + 1) << 1;
        self->l.data = memcpy(malloc(cap), &self->s, self->length + 1);
        self->l.capacity = cap;
        self->length |= STR_LONGBIT;
    }
    else if(!str_issmall(self) && len >= self->l.capacity) {
        // Long String without enough capacity
        char* olddata = self->l.data;
        self->l.capacity = self->l.capacity << 1;
        self->l.data =
            memcpy(malloc(self->l.capacity), olddata, self->length + 1);
        free(olddata);
    }
}

// NOLINTEND

Str str_create(const char* s) {
    if(s) return str_create_n(s, 0, strlen(s));
    return (Str){.length = 0};
}

Str str_create_n(const char* s, size_t start, size_t n) {
    Str self = {.length = n};

    if(!STR_CHECKSMALL(&self, self.length)) {
        self.l.capacity = (self.length + 1) << 1;
        self.l.data = memcpy(malloc(self.l.capacity), s + start, self.length);
        self.l.data[self.length] = 0;
        self.length |= STR_LONGBIT;
    }
    else memcpy(self.s, s, self.length);

    return self;
}

void str_append_n(Str* self, const char* s, size_t n) {
    _str_checkcapacity(self, n);

    size_t len = str_getlength(self);
    size_t newlen = len + n;

    if(str_issmall(self)) {
        memcpy(self->s + len, s, n);
        self->s[newlen] = 0;
        self->length = newlen;
    }
    else {
        memcpy(self->l.data + len, s, n);
        self->l.data[newlen] = 0;
        self->length = STR_LONGBIT | newlen;
    }
}

void str_append(Str* self, const char* s) {
    if(s) str_append_n(self, s, strlen(s));
}

void str_appendstr(Str* self, const Str* s) {
    if(s) str_append_n(self, str_getdata(s), str_getlength(s));
}

bool str_startswithstr(Str* self, const Str* s) {
    return self && s &&
           str_startswith_n(self, str_getdata(s), str_getlength(s));
}

bool str_startswith_n(Str* self, const char* s, size_t n) {
    return self && s &&
           _cstr_startswith(str_getdata(self), str_getlength(self), s, n);
}

bool str_startswith(Str* self, const char* s) {
    return self && s && str_startswith_n(self, s, strlen(s));
}

bool str_endswithstr(Str* self, const Str* s) {
    return self && s && str_endswith_n(self, str_getdata(s), str_getlength(s));
}

bool str_endswith_n(Str* self, const char* s, size_t n) {
    return self && s &&
           _cstr_endswith(str_getdata(self), str_getlength(self), s, n);
}

bool str_endswith(Str* self, const char* s) {
    return self && s && str_endswith_n(self, s, strlen(s));
}

int str_indexofstr(Str* self, const Str* s) {
    if(!self || !s) return -1;
    return str_indexof_n(self, str_getdata(s), str_getlength(s));
}

int str_indexof_n(Str* self, const char* s, size_t n) {
    if(!self) return -1;
    return _cstr_indexof(str_getdata(self), str_getlength(self), s, n);
}

int str_indexof(Str* self, const char* s) {
    if(!self) return -1;
    return s && str_indexof_n(self, s, strlen(s));
}

int str_lastindexofstr(Str* self, const Str* s) {
    if(!self || !s) return -1;
    return str_lastindexof_n(self, str_getdata(s), str_getlength(s));
}

int str_lastindexof_n(Str* self, const char* s, size_t n) {
    if(!self || !s) return -1;
    return _cstr_lastindexof(str_getdata(self), str_getlength(self), s, n);
}

int str_lastindexof(Str* self, const char* s) {
    if(!self) return -1;
    return self && s && str_lastindexof_n(self, s, strlen(s));
}

bool str_issmall(const Str* self) {
    return !self || !(self->length & STR_LONGBIT);
}

bool str_isempty(const Str* self) { return !self || !str_getlength(self); }

size_t str_getlength(const Str* self) {
    if(!self) return 0;
    return self->length & STR_LENMASK;
}

size_t str_getcapacity(const Str* self) {
    if(!self) return 0;
    return str_issmall(self) ? sizeof(self->s) : self->l.capacity;
}

const char* str_getdata(const Str* self) {
    if(!self) return NULL;
    return str_issmall(self) ? self->s : self->l.data;
}

bool str_equals(const Str* self, const Str* rhs) {
    if(!self || !rhs) return false;
    const char* pl = str_issmall(self) ? self->s : self->l.data;
    const char* pr = str_issmall(rhs) ? rhs->s : rhs->l.data;
    return !strcmp(pl, pr);
}

void str_clear(Str* self) {
    if(!self || !str_getlength(self)) return;

    if(str_issmall(self)) {
        self->length = 0;
        self->s[0] = 0;
    }
    else if(self->l.data) {
        self->length = STR_LONGBIT;
        self->l.data[0] = 0;
    }
}

void str_destroy(Str* self) {
    if(!self) return;

    if(!str_issmall(self)) {
        free(self->l.data);
        self->l.data = NULL;
        self->l.capacity = 0;
    }

    self->length = 0;
}
