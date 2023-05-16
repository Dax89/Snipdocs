#pragma once

#include <cstring>
#include <type_traits>
#include <algorithm>
#include <limits>
#include <optional>
#include <random>
#include <array>
#include <string>
#include "error.h"

#if defined(__unix__)
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
#else
    #error "Unsupported operating system"
#endif

namespace impl {

template<typename> constexpr bool always_false_v = false;
const std::string HASH_SUFFIX = ".hash";
const std::string VALUE_SUFFIX = ".value";

#if defined(_WIN32)
    constexpr std::string_view PATH_SEPARATOR = "\\";
#else
    constexpr std::string_view PATH_SEPARATOR = "/";
#endif

#if defined(__unix__)
    using file_h = int;
    using file_o = off_t;
    constexpr file_h INVALID_HANDLE = -1;
#endif

inline file_h is_file(const std::string& filepath) {
#if defined(__unix__)
    return ::access(filepath.c_str(), F_OK) == 0;
#endif

}

inline file_h open(const std::string& filepath) {
#if defined(__unix__)
    file_h h = ::open(filepath.c_str(), O_RDWR | O_CREAT, 0644);
    assume(h != -1);
    return h;
#endif
}

inline void close(file_h h) {
#if defined(__unix__)
    ::close(h);
#endif
}

inline file_o tell(file_h h) {
#if defined(__unix__)
    return ::lseek(h, 0, SEEK_CUR);
#endif
}

inline file_o seek_end(file_h h) {
#if defined(__unix__)
    return ::lseek(h, 0, SEEK_END);
#endif
}

inline file_o seek(file_h h, size_t offset) {
#if defined(__unix__)
    return ::lseek(h, offset, SEEK_SET);
#endif
}

inline size_t write(file_h h, const void* data, size_t nbytes) {
#if defined(__unix__)
    ssize_t s = ::write(h, data, nbytes);
    assume(s != -1);
    assume(static_cast<size_t>(s) == nbytes);
    return static_cast<size_t>(s);
#endif
}

inline void read(file_h h, void* data, size_t nbytes) {
#if defined(__unix__)
    assume(static_cast<size_t>(::read(h, data, nbytes)) == nbytes);
#endif
}

inline file_o size(file_h h) {
    file_o o = impl::tell(h);
    impl::seek_end(h);
    file_o s = impl::tell(h);
    impl::seek(h, o);
    return s;
}

inline void resize(file_h h, size_t s) {
#if defined(__unix__)
    ::ftruncate(h, s);
#endif
}

template<typename T>
inline T* mmap(file_h h, size_t size) {
#if defined(__unix__)
    return reinterpret_cast<T*>(::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, h, 0));
#endif

}

inline void munmap(void* m, [[maybe_unused]] size_t size) {
#if defined(__unix__)
    ::munmap(m, size);
#endif
}

inline size_t fnv1a(const void* data, size_t size) {
    constexpr size_t FNV_OFFSET_BASIS = [](){
        if constexpr(sizeof(size_t) == sizeof(uint64_t)) return 14695981039346656037ULL;
        else return 2166136261U;
    }();

    constexpr size_t FNV_PRIME = [](){
        if constexpr(sizeof(size_t) == sizeof(uint64_t)) return 1099511628211ULL;
        else return 16777619U;
    }();

    size_t h = FNV_OFFSET_BASIS;
    const unsigned char* p = static_cast<const unsigned char*>(data);
    const unsigned char* const end = p + size;

    for( ; p != end; ++p) {
        h ^= static_cast<size_t>(*p);
        h *= FNV_PRIME;
    }

    return h;
}

inline size_t fnv1a(std::string_view value) { return impl::fnv1a(value.data(), value.size()); }

inline size_t fnv1a(float value) {
    static_assert(std::numeric_limits<float>::is_iec559,
        "fnv1a is only defined for IEEE 754-compliant floating-point types");
    static_assert(sizeof(value) == sizeof(uint32_t),
        "fnv1a is only defined for 32-bit floating-point types");

    uint32_t bits;
    std::copy_n(&value, sizeof(bits), &bits);
    return impl::fnv1a(&bits, sizeof(bits));
}

inline size_t fnv1a(double value) {
    static_assert(std::numeric_limits<double>::is_iec559,
        "fnv1a is only defined for IEEE 754-compliant floating-point types");
    static_assert(sizeof(value) == sizeof(uint64_t),
        "fnv1a is only defined for 64-bit floating-point types");

    uint64_t bits;
    std::copy_n(&value, sizeof(bits), &bits);
    return impl::fnv1a(&bits, sizeof(bits));
}

struct Serializer {
    template<typename T, typename Reader>
    static void deserialize(T& t, Reader r) {
        if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>)
            r(reinterpret_cast<void*>(&t), sizeof(T));
        else if constexpr(std::is_same_v<T, std::string>) {
            std::string::size_type size;
            r(reinterpret_cast<void*>(&size), sizeof(std::string::size_type));

            t.resize(size);
            r(reinterpret_cast<void*>(t.data()), size);
        }
        else
            t.deserialize(r);
    }

    template<typename T, typename Writer>
    static void serialize(T&& t, Writer w) {
        if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>)
            w(reinterpret_cast<void*>(&t), sizeof(T));
        else if constexpr(std::is_same_v<T, std::string>) {
            std::string::size_type size = t.size();
            w(reinterpret_cast<void*>(&size), sizeof(std::string::size_type));
            w(t.c_str(), t.size());
        }
        else
            t.serialize(w);
    }
};

} // namespace impl

enum disk_map_flags {
    disk_map_flags_none  = 0,
    disk_map_flags_split = (1 << 0),
    disk_map_flags_remove = (1 << 1),
};

template<typename K, typename V, size_t Flags = disk_map_flags_none, typename Serializer = impl::Serializer>
class disk_map
{
    using Self = disk_map<K, V, Flags, Serializer>;

    static constexpr bool SPLIT_VALUE = (Flags & disk_map_flags_split) || (sizeof(V) > sizeof(uintptr_t));
    static constexpr size_t SIGNATURE = 0x5d1b0239;
    static constexpr size_t DEFAULT_ITEMS_COUNT = 4096;
    static constexpr float MAX_FILL_CAPACITY = 0.75;

    enum {
        STATE_EMPTY = 0,
        STATE_TOMBSTONE,
        STATE_FULL,
    };

    struct hash_offset_value {
        size_t capacity;
        size_t offset;
    };

    struct kv_pair {
        size_t state;
        K key;
        std::conditional_t<SPLIT_VALUE, hash_offset_value, V> value;
    };

    struct hash_header {
        unsigned char integersize;
        size_t signature;
        size_t capacity;
        size_t size;
        size_t fill;
        size_t valuecapacity;
        size_t valuesize;
    };

    struct value_getter {
        value_getter(const Self* s, const kv_pair* e): m_self{s}, m_e{e} { }

        V operator*() const {
            assume(m_e && m_e->state == STATE_FULL);

            V v;
            assume(m_self->get_value(*m_e, v));
            return v;
        }

    private:
        const Self* m_self;
        const kv_pair* m_e;
    };

    struct iterator {
        iterator(const Self* s, const kv_pair* e, const kv_pair* ee): m_self{s}, m_e{e}, m_ende{ee} { }
        K key() const { return m_e->key; }
        V value() const { return *value_getter{m_self, m_e}; }

        iterator& operator++() {
            if(m_e != m_ende) {
                ++m_e;
                while(m_e != m_ende && m_e->state != STATE_FULL) ++m_e;
            }

            return *this;
        }

        iterator operator++(int) {
            iterator it = *this;
            ++(*this);
            return it;
        }

        std::pair<K, value_getter> operator *() const { return {m_e->key, value_getter{m_self, m_e}}; }
        bool operator ==(const iterator& rhs) const { return m_self == rhs.m_self && m_e == rhs.m_e; }
        bool operator !=(const iterator& rhs) const { return m_self != rhs.m_self || m_e != rhs.m_e;  }

    private:
        const Self* m_self;
        const kv_pair *m_e, *m_ende;
    };

public:
    disk_map(const std::string& name, std::string basepath = std::string{}) {
        assume(!name.empty());
        if(!basepath.empty()) basepath.append(impl::PATH_SEPARATOR);

        m_fhashpath = basepath + name + ".hash";
        this->reinit_hashfile(DEFAULT_ITEMS_COUNT, true);

        m_hash->integersize = sizeof(size_t);
        m_hash->signature = SIGNATURE;
        m_hash->capacity = DEFAULT_ITEMS_COUNT;
        m_hash->valuesize = 0;
        m_hash->size = 0;
        m_hash->fill = 0;

        if constexpr(SPLIT_VALUE) {
            m_fvaluepath = basepath + name + impl::VALUE_SUFFIX;
            m_hash->valuecapacity = DEFAULT_ITEMS_COUNT * sizeof(V);
            this->reinit_valuefile(m_hash->valuecapacity);
        }
        else
            m_hash->valuecapacity = 0;
    }

    ~disk_map() {
        if(m_hash) impl::munmap(m_hash, m_hash->capacity * sizeof(kv_pair));
        if(m_fhash != impl::INVALID_HANDLE) impl::close(m_fhash);
        if(m_fvalue != impl::INVALID_HANDLE) impl::close(m_fvalue);

        m_fhash = impl::INVALID_HANDLE;
        m_fvalue = impl::INVALID_HANDLE;

        if constexpr(Flags & disk_map_flags_remove) {
            if(!m_fvaluepath.empty()) std::remove(m_fvaluepath.c_str());
            if(!m_fhashpath.empty()) std::remove(m_fhashpath.c_str());
        }
    }

    iterator begin() const {
        kv_pair* e = this->get_kvpairs();
        kv_pair* ee = this->get_kvpairs() + m_hash->capacity;
        while(e != ee && e->state != STATE_FULL) e++;
        return iterator{this, e, ee};
    }

    iterator end() const {
        kv_pair* e = this->get_kvpairs() + m_hash->capacity;
        return iterator{this, e, e};
    }

    float load_factor() { return static_cast<float>(m_hash->fill) / static_cast<float>(m_hash->capacity); }
    size_t capacity() const { return m_hash->capacity; }
    size_t size() const { return m_hash->size; }
    bool empty() const { return m_hash->size == 0; }

    bool contains(K k) const {
        const kv_pair& e = this->get_entry(k);
        return e.state == STATE_FULL;
    }

    void clear() {
        kv_pair* kv = this->get_kvpairs();
        std::fill_n(reinterpret_cast<char*>(kv), m_hash->capacity * sizeof(kv_pair), 0);
        m_hash->fill = m_hash->size = m_hash->valuesize = 0;
    }

    void erase(K k) {
        kv_pair& e = this->get_entry(k);
        if(e.state != STATE_FULL) return;
        --m_hash->size;
        e.state = STATE_TOMBSTONE;
    }

    void set(K k, V&& v) {
        this->check_rehash();

        kv_pair& e = this->get_entry(k);
        e.key = k;

        if(e.state != STATE_FULL) ++m_hash->size;
        if(e.state == STATE_EMPTY) ++m_hash->fill;

        if constexpr(SPLIT_VALUE) {
            size_t n = 0;
            m_wbuffer.clear();

            Serializer::serialize(std::forward<V>(v), [&](const void* data, size_t size) {
                m_wbuffer.resize(m_wbuffer.size() + size);
                std::copy_n(reinterpret_cast<const char*>(data), size, m_wbuffer.data() + n);
                n += size;
            });

            if(e.state == STATE_EMPTY || n > e.value.capacity) {
                if(this->values_filled() > MAX_FILL_CAPACITY) this->extend_value();
                e.value.capacity = n;
                e.value.offset = impl::seek(m_fvalue, m_hash->valuesize);
                m_hash->valuesize += n;
            }
            else
                impl::seek(m_fvalue, e.value.offset);

            impl::write(m_fvalue, m_wbuffer.data(), n);
        }
        else
            e.value = v;

        e.state = STATE_FULL;
    }

    bool get(K k, V& v) const {
        if(this->empty()) return false;
        const kv_pair& e = this->get_entry(k);
        return this->get_value(e, v);
    }

    std::optional<V> get(K k) const {
        V v;
        if(this->get(k, v)) return v;
        return std::nullopt;
    }

    void collect_garbage() {
        if(this->empty()) return;

        this->check_rehash();

        if constexpr(SPLIT_VALUE) {
            std::string tmpvalue = m_fvaluepath + ".tmp";
            impl::file_h newfile = impl::open(tmpvalue);
            assume(newfile != impl::INVALID_HANDLE);
            impl::resize(newfile, m_hash->valuecapacity);

            kv_pair* e = this->get_kvpairs();
            impl::file_o offset = 0;

            for(size_t i = 0; i < m_hash->capacity; ++i, ++e) {
                if(e->state != STATE_FULL) continue;

                if(m_wbuffer.size() < e->value.capacity)
                    m_wbuffer.resize(e->value.capacity);

                impl::seek(m_fvalue, e->value.offset);
                impl::read(m_fvalue, m_wbuffer.data(), e->value.capacity);
                e->value.offset = offset;

                impl::write(newfile, m_wbuffer.data(), e->value.capacity);
                offset += e->value.capacity;
            }

            m_hash->valuesize = offset;
            impl::close(newfile);

            // Close and delete the old file, rename the new one
            impl::close(m_fvalue);
            std::remove(m_fvaluepath.c_str());
            std::rename(tmpvalue.c_str(), m_fvaluepath.c_str());
            this->reinit_valuefile(m_hash->valuecapacity);
        }
    }

    void rehash() {
        assume(!m_fhashpath.empty());
        assume(m_fhash != impl::INVALID_HANDLE);
        assume(m_hash);

        size_t newcapacity = m_hash->capacity << 1;
        size_t newsize = sizeof(hash_header) + (newcapacity * sizeof(kv_pair));
        std::string tmphash = m_fhashpath + ".tmp";
        impl::file_h newfile = impl::open(tmphash);
        assume(newfile != impl::INVALID_HANDLE);
        impl::resize(newfile, newsize);

        hash_header* newhash = impl::mmap<hash_header>(newfile, newsize);
        *newhash = *m_hash;
        newhash->capacity = newcapacity;
        newhash->fill = newhash->size; // Reset tombstones count

        kv_pair* newpair = reinterpret_cast<kv_pair*>(newhash + 1);
        kv_pair* oldpair = this->get_kvpairs();

        for(size_t i = 0; i < m_hash->capacity; ++i, ++oldpair) {
            if(oldpair->state != STATE_FULL) continue;

            size_t idx = this->hash(oldpair->key) % newhash->capacity;
            newpair[idx] = *oldpair;
        }

        impl::munmap(newhash, newsize);
        impl::close(newfile);

        // Unmap, close and delete the old file, rename the new one
        size_t oldsize = sizeof(hash_header) + (m_hash->capacity * sizeof(kv_pair));
        impl::munmap(m_hash, oldsize);
        impl::close(m_fhash);
        std::remove(m_fhashpath.c_str());
        std::rename(tmphash.c_str(), m_fhashpath.c_str());
        this->reinit_hashfile(newcapacity);
    }

    static Self load(const std::string& name, std::string basepath = std::string{}) {
        assume(!name.empty());
        if(!basepath.empty()) basepath.append(impl::PATH_SEPARATOR);

        std::string hashpath = basepath + name + impl::HASH_SUFFIX;
        if(!impl::is_file(hashpath)) except("Hash file '{}' not found", hashpath);
        return Self{impl::open(hashpath), name, basepath};
    }

private:
    disk_map(impl::file_h fhash, [[maybe_unused]] const std::string& name, [[maybe_unused]] const std::string basepath): m_fhash{fhash} {
        assume(m_fhash != impl::INVALID_HANDLE);
        m_fhashpath = basepath + impl::HASH_SUFFIX;

        size_t size = impl::size(fhash);
        m_hash = impl::mmap<hash_header>(m_fhash, size);
        assume(m_hash);

        if(m_hash->integersize != sizeof(size_t)) except("Unexpected integer size");
        if(m_hash->signature != SIGNATURE) except("Invalid signature");

        if constexpr(SPLIT_VALUE) {
            m_fvaluepath = basepath + name + impl::VALUE_SUFFIX;
            if(!impl::is_file(m_fvaluepath)) except("Value file '{}' not found", m_fvaluepath);
            m_fvalue = impl::open(m_fvaluepath);
            assume(m_fvalue != impl::INVALID_HANDLE);
        }
    }

    kv_pair* get_kvpairs() const { return reinterpret_cast<kv_pair*>(m_hash + 1); }
    float values_filled() { return static_cast<float>(m_hash->valuesize) / static_cast<float>(m_hash->valuecapacity); }

    bool get_value(const kv_pair& e, V& v) const {
        if(e.state != STATE_FULL) return false;

        if constexpr(SPLIT_VALUE) {
            impl::seek(m_fvalue, e.value.offset);

            Serializer::deserialize(v, [&](void* data, size_t size) {
                impl::read(m_fvalue, data, size);
            });
        }
        else
            v = e.value;

        return true;
    }

    size_t hash(K k) const {
        if constexpr(std::is_integral_v<K>) return k;
        else if constexpr(std::is_floating_point_v<K> || std::is_same_v<K, std::string>) return impl::fnv1a(k);
        else static_assert(impl::always_false_v<K>);
    }

    const kv_pair& get_entry(K k) const { return const_cast<Self*>(this)->get_entry(k); }

    kv_pair& get_entry(K k) {
        kv_pair* h = this->get_kvpairs();

        for(size_t index = this->hash(k) % m_hash->capacity; ; index = (index + 1) % m_hash->capacity) {
            if(h[index].state != STATE_FULL || h[index].key == k)
                return h[index];
        }

        unreachable;
    }

    void extend_value() {
        assume(m_fvalue != impl::INVALID_HANDLE);
        m_hash->valuecapacity <<= 1;
        impl::resize(m_fvalue, m_hash->valuecapacity);
    }

    void check_rehash() {
        if(this->load_factor() > MAX_FILL_CAPACITY)
            this->rehash();
    }

    void reinit_hashfile(size_t capacity = DEFAULT_ITEMS_COUNT, bool init = false) {
        assume(!m_fhashpath.empty());
        size_t size = sizeof(hash_header) + (capacity * sizeof(kv_pair));

        m_fhash = impl::open(m_fhashpath);
        assume(m_fhash != impl::INVALID_HANDLE);

        impl::resize(m_fhash, size);
        m_hash = impl::mmap<hash_header>(m_fhash, size);
        if(init) std::fill_n(reinterpret_cast<char*>(m_hash), size, 0);
        assume(m_hash);
    }

    void reinit_valuefile(size_t capacity = DEFAULT_ITEMS_COUNT) {
        assume(!m_fvaluepath.empty());
        m_fvalue = impl::open(m_fvaluepath);
        assume(m_fvalue != impl::INVALID_HANDLE);
        impl::resize(m_fvalue, capacity);
    }

private:
    std::string m_fhashpath;
    std::string m_fvaluepath;
    std::string m_wbuffer;
    impl::file_h m_fhash{impl::INVALID_HANDLE};
    impl::file_h m_fvalue{impl::INVALID_HANDLE};
    hash_header* m_hash{nullptr};
};
