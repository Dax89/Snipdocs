#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// NOLINTBEGIN
#define map_default_size 256
#define map_max_loadfactor 0.75f

enum {
    MS_TOMB = -1,
    MS_NULL = 0,
    MS_FULL = 1,
};

#define map_bucket_fields                                                      \
    uintptr_t hash;                                                            \
    ptrdiff_t state

// clang-format off
typedef struct MapBucket { map_bucket_fields; } MapBucket;
#define Map(KV) KV*

#define map_header(self) ((MapHeader*)(((char*)(self)) - offsetof(MapHeader, buckets)))
#define map_create_full(KV, n, hashfn, equalsfn) (Map(KV))_map_create_n(sizeof(KV), n, hashfn, equalsfn) 
#define map_create_n(KV, n) map_create_full(KV, n, _map_hash__##KV, _map_equals__##KV) 
#define map_create(KV) map_create_n(KV, map_default_size) 
#define map_begin(self) (self)
#define map_end(self) ((self) + map_header(self)->capacity)
#define map_length(self) (map_header(self)->length)
#define map_capacity(self) (map_header(self)->capacity)
#define map_loadfactor(self) ((float)(map_header(self)->length + map_header(self)->tombs) / (float)map_header(self)->capacity)
#define map_empty(self) (!(self) || !map_header(self)->length)
#define map_destroy(self) if(self) free(map_header(self))
#define map_getbucket(KV, self, k) _map_getbucket__##KV(self, k)
#define map_get(KV, self, k) _map_get__##KV(self, k)
#define map_set(KV, self, k, v) self = _map_set__##KV(self, k, v)
#define map_del(KV, self, k) self = _map_del__##KV(self, k)
#define map_clear(KV, self) _map_clear(self, sizeof(KV))
#define map_contains(KV, self, k) (map_get(KV, self, K) != NULL)
#define map_rehash(KV, self, n) self = _map_rehash__##KV(self, n)
#define map_reserve(KV, self, n) self = _map_rehash__##KV(self, ceil(n / map_max_loadfactor))
#define map_sethash(KV, hashfn) map_header(self)->hash = hashfn
#define map_setequals(KV, equalsfn) map_header(self)->equals = equalsfn
#define map_itemdestroy(KV, itemdestroyfn) map_header(self)->itemdestroy = itemdestroyfn

#define map_foreach(KV, item, self)                                             \
    for(Map(KV) item = map_begin(self); item != map_end(self); ++item)          \
        if(item->state == MS_FULL)

#define _MapItemType(KV, K, V)                                                  \
    typedef K KV##Key;                                                          \
    typedef V KV##Value;                                                        \
    typedef struct KV { map_bucket_fields; K key; V value; } KV;                \
    inline Map(KV) _map_rehash__##KV(Map(KV) self, uintptr_t n) {               \
        return (Map(KV))_map_rehash(self, sizeof(KV), sizeof(K), n);            \
    }                                                                           \
    inline const Map(KV) _map_getbucket__##KV(const Map(KV) self, K k) {        \
        uintptr_t idx =                                                         \
        _map_findbucket(self, &k, sizeof(K), sizeof(KV), false);                \
        if(self[idx].state == MS_FULL) return &self[idx];                       \
        return NULL;                                                            \
    }                                                                           \
    inline Map(KV) _map_set__##KV(Map(KV) self, K k, V v) {                     \
        self = (KV*)_map_check_rehash(self, sizeof(KV), sizeof(K));             \
        uintptr_t idx =                                                         \
            _map_findbucket(self, &k, sizeof(K), sizeof(KV), true);             \
        self[idx].key = k;                                                      \
        self[idx].value = v;                                                    \
        return self;                                                            \
    }                                                                           \
    inline const V* _map_get__##KV(const Map(KV) self, K k) {                   \
        const KV* b = _map_getbucket__##KV(self, k);                            \
        if(b) return &b->value;                                                 \
        return NULL;                                                            \
    }                                                                           \
    inline bool _map_del__##KV(Map(KV) self, K k) {                             \
        uintptr_t idx =                                                         \
            _map_findbucket(self, &k, sizeof(K), sizeof(KV), false);            \
        if(self[idx].state != MS_FULL) return false;                            \
        MapHeader* hdr = map_header(self);                                      \
        if(hdr->itemdestroy) hdr->itemdestroy((MapBucket*)&self[idx]);          \
        self[idx].state = MS_TOMB;                                              \
        hdr->length--;                                                          \
        hdr->tombs++;                                                           \
        return true;                                                            \
    }

#define MapItem(KV, K, V)                                                       \
    _MapItemType(KV, K, V);                                                     \
    inline bool _map_equals__##KV(const void* key1, const void* key2,           \
                                  uintptr_t) {                                  \
        return (*(const KV##Key*)key1) == (*(const KV##Key*)key2);              \
    }                                                                           \
    inline uintptr_t _map_hash__##KV(const void* k, uintptr_t) {                \
        return _map_fnv1a(k, sizeof(K));                                        \
    }

#define MapItemStr(KV, K, V)                                                    \
    _MapItemType(KV, K, V);                                                     \
    inline bool _map_equals__##KV(const void* key1, const void* key2,           \
                                  uintptr_t) {                                  \
        return !strcmp(*((const KV##Key**)key1), *((const KV##Key**)key2));     \
    }                                                                           \
    inline uintptr_t _map_hash__##name(const void* k, uintptr_t) {              \
        const char* str = *((const char**)k);                                   \
        return _map_fnv1a(str, strlen(str));                                    \
    }

struct MapHeader;

typedef uintptr_t (*MapHash)(const void*, uintptr_t);
typedef uintptr_t (*MapProbe)(const MapHeader*, uintptr_t);
typedef bool (*MapEquals)(const void*, const void*, uintptr_t);
typedef void (*MapItemDestroy)(MapBucket*);
// clang-format on

typedef struct MapHeader {
    uintptr_t length;
    uintptr_t tombs;
    uintptr_t capacity;
    MapHash hash;
    MapProbe probe;
    MapEquals equals;
    MapItemDestroy itemdestroy;
    char buckets[1];
} MapHeader;

#if UINTPTR_MAX == 0xFFFFFFFF // 32-bit platform
    #define map_fnv1a_prime1 2166136261U
    #define map_fnv1a_prime2 16777619
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF // 64-bit platform
    #define map_fnv1a_prime1 14695981039346656037ULL
    #define map_fnv1a_prime2 1099511628211
#else
    #error "Unknown platform"
#endif

static uintptr_t _map_fnv1a(const void* data, uintptr_t len) {
    const uint8_t* p = (const uint8_t*)(data);
    uintptr_t hash = map_fnv1a_prime1;

    for(uintptr_t i = 0; i < len; i++) {
        hash ^= (uintptr_t)(p[i]);
        hash *= map_fnv1a_prime2;
    }

    return hash;
}

inline uintptr_t _map_findbucket(const Map(void) self, const void* key,
                                 uintptr_t keysize, uintptr_t bucketsize,
                                 bool w) {
    MapHeader* hdr = map_header(self);
    uintptr_t h = hdr->hash(key, keysize), idx = h % hdr->capacity,
              tombidx = (uintptr_t)-1;

    for(MapBucket* citem = NULL;; idx = hdr->probe(hdr, idx)) {
        citem = (MapBucket*)((char*)self + (idx * bucketsize));
        const void* ckey = citem + 1;

        // Case 1: Found a valid item that matches
        if(citem->state == MS_FULL && h == citem->hash &&
           hdr->equals(key, ckey, keysize))
            return idx;

        // Case 2: Writing on map
        if(w) {
            if(citem->state == MS_TOMB && tombidx == (uintptr_t)-1)
                tombidx = idx; // Store first available tomb

            if(citem->state == MS_NULL) {
                // Use tomb if available, otherwise use the empty slot
                if(tombidx != (uintptr_t)-1) {
                    idx = tombidx;
                    hdr->tombs--;
                }
                else hdr->length++;

                citem->hash = h;
                citem->state = MS_FULL;
                return idx;
            }
        } // Case 3: Read or Delete (Search)
        else if(citem->state == MS_NULL) return idx;
    }

    // Should never reach here, but prevents undefined behavior
    assert(0 && "Unreachable code in _map_getindex()");
    return idx;
}

inline uintptr_t _map_defaultprobe(const MapHeader* self, uintptr_t idx) {
    return (uintptr_t)((idx + 1) % map_header(self)->capacity);
}

inline Map(void) _map_create_n(uintptr_t bucketsize, uintptr_t cap,
                               MapHash hashfn, MapEquals equalsfn) {
    if(!bucketsize || !cap) return NULL;

    MapHeader* hdr =
        (MapHeader*)calloc(1, sizeof(MapHeader) + (bucketsize * cap));
    if(!hdr) return NULL;

    hdr->length = 0;
    hdr->capacity = cap;
    hdr->hash = hashfn;
    hdr->probe = _map_defaultprobe;
    hdr->equals = equalsfn;
    hdr->itemdestroy = NULL;
    return hdr->buckets;
}

inline Map(void) _map_rehash(Map(void) self, uintptr_t bucketsize,
                             uintptr_t keysize, uintptr_t newcap) {
    MapHeader* hdr = map_header(self);
    if(!newcap) newcap = hdr->capacity; // Just rehash
    else if(hdr->capacity >= newcap) return self;

    MapHeader* newhdr =
        (MapHeader*)calloc(1, sizeof(MapHeader) + (bucketsize * newcap));

    *newhdr = *hdr;
    newhdr->capacity = newcap;
    newhdr->tombs = 0;
    newhdr->length = 0;
    newhdr->buckets[0] = 0;

    Map(void) newself = &newhdr->buckets;
    const MapBucket* cbucket = NULL;

    for(uintptr_t len = hdr->length, i = 0; len > 0; i++) {
        cbucket = (const MapBucket*)((char*)self + (i * bucketsize));
        if(cbucket->state != MS_FULL) continue;
        const void* ckey = cbucket + 1;

        uintptr_t idx =
            _map_findbucket(newself, ckey, keysize, bucketsize, true);
        MapBucket* bucket = (MapBucket*)((char*)newself + (idx * bucketsize));
        memcpy(bucket, cbucket, bucketsize);
        --len;
    }

    free(hdr);
    return newself;
}

void _map_clear(Map(void) self, uintptr_t bucketsize) {
    MapHeader* hdr = map_header(self);

    for(char* it = (char*)self;
        it != (char*)self + (hdr->capacity * bucketsize) &&
        (hdr->length || hdr->tombs);
        it += bucketsize) {
        MapBucket* b = (MapBucket*)it;

        if(b->state == MS_FULL) {
            if(hdr->itemdestroy) hdr->itemdestroy(b);
            --hdr->length;
        }
        else if(b->state == MS_TOMB) --hdr->tombs;

        b->state = MS_NULL;
    }

    assert(hdr->length == 0 && "Length is not 0 in _map_clear()");
    assert(hdr->tombs == 0 && "Tombs is not 0 in _map_clear()");
}

inline Map(void)
    _map_check_rehash(Map(void) self, uintptr_t bucketsize, uintptr_t n) {
    if(map_loadfactor(self) >= map_max_loadfactor) {
        return _map_rehash(self, bucketsize, n,
                           map_header(self)->capacity << 1);
    }

    return self;
}
// NOLINTEND

#if defined(__cplusplus)
}
#endif
