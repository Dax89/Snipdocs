#pragma once

// https://github.com/msgpack/msgpack/blob/master/spec.md

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <functional>
#include <variant>
#include <algorithm>
#include <sys/param.h>
#include <type_traits>
#include <string>
#include <string_view>
#include <array>
#include <map>
#include <unordered_map>
#include <vector>
#include <limits>

#if defined(__EXCEPTIONS) || defined(_CPPUNWIND)
    #include <stdexcept>
#endif

#if defined(__BYTE_ORDER)
    #if defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)
      #define MSGPACK_BIG_ENDIAN
    #elif defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)
      #define MSGPACK_LITTLE_ENDIAN
    #else
        static_assert(false, "MsgPack: Unsupported endianness")
    #endif
#else 
    static_assert(false, "MsgPack: Cannot detect byte order")
#endif

namespace msgpack {

struct Visitor {
    using integer_type = std::variant<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>;

    bool start_map(size_t /* size */) { return true; }
    bool end_map() { return true; }
    bool start_map_key(size_t /* index */) { return true;}
    bool end_map_key(size_t /* index */) { return true; }
    bool start_map_value(size_t /* index */) { return true; }
    bool end_map_value(size_t /* index */) { return true; }
    bool start_array(size_t /* size */) { return true; }
    bool end_array() { return true; }
    bool start_array_item(size_t /* index */) { return true; }
    bool end_array_item(size_t /* index */) { return true; }
    bool visit_nil() { return true; }
    bool visit_bool(bool /* arg */) { return true; }
    bool visit_str(std::string_view /* arg */) { return true; }
    bool visit_int(integer_type /* arg */) { return true; }
};

namespace impl {

template<typename>
inline constexpr bool always_false_v = false;

template<typename T>
inline constexpr bool is_bool_v = std::is_same_v<T, bool>;

template<typename T>
inline constexpr bool is_array_v = false;

template<typename T, typename Allocator>
inline constexpr bool is_array_v<std::vector<T, Allocator>> = true;

template<typename T, std::size_t N>
inline constexpr bool is_array_v<std::array<T, N>> = true;

template<typename T>
inline constexpr bool is_vector_v = false;

template<typename T, typename Allocator>
inline constexpr bool is_vector_v<std::vector<T, Allocator>> = true;

template<typename T>
inline constexpr bool is_map_v = false;

template<typename K, typename V, typename Compare, typename Allocator>
inline constexpr bool is_map_v<std::map<K, V, Compare, Allocator>> = true;

template<typename K, typename V, typename Compare, typename Allocator>
inline constexpr bool is_map_v<std::unordered_map<K, V, Compare, Allocator>> = true;

template<typename T>
inline constexpr bool is_string_v = false;

template<>
inline constexpr bool is_string_v<std::string> = true;

template<>
inline constexpr bool is_string_v<std::string_view> = true;

template<>
inline constexpr bool is_string_v<const char*> = true;

namespace format {

enum: uint8_t {
    NIL      = 0xc0,
    FIXMAP   = 0x80,
    FIXARRAY = 0x90,
    FIXSTR   = 0xa0,
    FALSE    = 0xc2,
    TRUE     = 0xc3,
    UINT8    = 0xcc,
    UINT16   = 0xcd,
    UINT32   = 0xce,
    UINT64   = 0xcf,
    INT8     = 0xd0,
    INT16    = 0xd1,
    INT32    = 0xd2,
    INT64    = 0xd3,
    STR8     = 0xd9,
    STR16    = 0xda,
    STR32    = 0xdb,
    ARRAY16  = 0xdc,
    ARRAY32  = 0xdd,
    MAP16    = 0xde,
    MAP32    = 0xdf,
};

} // namespace format

[[noreturn]] static void msgpack_except(const char* errmsg) {
#if defined(__EXCEPTIONS) || defined(_CPPUNWIND)
    throw std::runtime_error(errmsg);
#elif !defined(_NDEBUG)
    assert(false)
#else 
    std::abort();
#endif
} 

template<typename T>
T swap_bigendian(T t) {
#if defined(MSGPACK_LITTLE_ENDIAN)
    union {
        T u;
        std::uint8_t b[sizeof(T)];
    } source{t}, dest;

    for(std::size_t i = 0; i < sizeof(T); i++)
        dest.b[i] = source.b[sizeof(T) - i - 1];

    return dest.u;
#else
    return t;
#endif
}

template<typename T, typename MsgPackType>
bool visit_type(MsgPackType& mp, T& t) {
    if(mp.at_end()) return false;
    mp.unpack(t);
    return true;
}

template<typename MsgPackType, typename VisitorType>
bool visit(MsgPackType& mp, VisitorType&& visitor);

template<typename MsgPackType, typename VisitorType>
bool visit_array(MsgPackType& mp, VisitorType&& visitor) {
    if(mp.at_end()) return false;

    size_t s = mp.unpack_array();
    if(!visitor.start_array(s)) return false;

    for(size_t i = 0; i < s; ++i) {
        if(!visitor.start_array_item(i) || !impl::visit(mp, std::move(visitor)) || !visitor.end_array_item(i)) return false;
    }

    return visitor.end_array();
}

template<typename MsgPackType, typename VisitorType>
bool visit_map(MsgPackType& mp, VisitorType&& visitor) {
    if(mp.at_end()) return false;

    size_t s = mp.unpack_map();
    if(!visitor.start_map(s)) return false;

    for(size_t i = 0; i < s; ++i) {
        if(!visitor.start_map_key(i) || !impl::visit(mp, std::move(visitor)) || !visitor.end_map_key(i)) return false;
        if(!visitor.start_map_value(i) || !impl::visit(mp, std::move(visitor)) || !visitor.end_map_value(i)) return false;
    }

    return visitor.end_map();
}

template<typename MsgPackType, typename VisitorType>
bool visit(MsgPackType& mp, VisitorType&& visitor) {
    if(mp.at_end()) return false;

    uint8_t f = static_cast<uint8_t>(mp.buffer.get()[mp.pos]);

    if((f & 0xF0) == impl::format::FIXMAP) return impl::visit_map(mp, visitor);
    else if((f & 0xF0) == impl::format::FIXARRAY) return impl::visit_array(mp, visitor);
    else if((f & 0xE0) == impl::format::FIXSTR) { std::string_view v; return impl::visit_type(mp, v) && visitor.visit_str(v); }

    switch (f) {
        case impl::format::MAP16:
        case impl::format::MAP32:  return impl::visit_map(mp, visitor);

        case impl::format::ARRAY16:
        case impl::format::ARRAY32:  return impl::visit_array(mp, visitor);

        case impl::format::STR8:
        case impl::format::STR16:
        case impl::format::STR32: { std::string_view v; return impl::visit_type(mp, v) && visitor.visit_str(v); }

        case impl::format::UINT8:  { uint8_t v; return impl::visit_type(mp, v) && visitor.visit_int(v); }
        case impl::format::UINT16: { uint16_t v; return impl::visit_type(mp, v) && visitor.visit_int(v); }
        case impl::format::UINT32: { uint32_t v; return impl::visit_type(mp, v) && visitor.visit_int(v); }
        case impl::format::UINT64: { uint64_t v; return impl::visit_type(mp, v) && visitor.visit_int(v); }
        case impl::format::INT8:   { int8_t v; return impl::visit_type(mp, v) && visitor.visit_int(v); }
        case impl::format::INT16:  { int16_t v; return impl::visit_type(mp, v) && visitor.visit_int(v); }
        case impl::format::INT32:  { int32_t v; return impl::visit_type(mp, v) && visitor.visit_int(v); }
        case impl::format::INT64:  { int64_t v; return impl::visit_type(mp, v) && visitor.visit_int(v); }
        case impl::format::TRUE:   { ++mp.pos; return visitor.visit_bool(true); }
        case impl::format::FALSE:  { ++mp.pos; return visitor.visit_bool(false); }
        case impl::format::NIL:    { ++mp.pos; return visitor.visit_nil(); }
        default: impl::msgpack_except("msgpack::visit(): Invalid Format"); break;
    }

    return false;
}

} // namespace impl

template<typename Container>
struct BasicMsgPack
{
    using type = BasicMsgPack<Container>;
    using container_type = Container;
    using value_type = typename container_type::value_type;

    static_assert(sizeof(value_type) == sizeof(uint8_t));

    explicit BasicMsgPack() = delete;
    explicit BasicMsgPack(container_type& c): buffer{c}, pos{} { }
    inline void push(const container_type& c) { std::copy(c.cbegin(), c.cend(), std::back_inserter(this->buffer.get())); }
    inline bool at_end() const { return this->pos >= this->buffer.get().size(); }
    inline void rewind() { this->pos = 0; }

    template<typename T>
    type& pack(T&& t) {
        using U = std::decay_t<T>;

        if constexpr(impl::is_array_v<U>) {
            this->pack_array(t.size());
            for(const auto& v : t) this->pack(v);
        }
        else if constexpr(impl::is_map_v<U>) {
            this->pack_map(t.size());
            for(const auto& [key, value] : t) {
                this->pack(key);
                this->pack(value);
            }
        }
        else if constexpr(impl::is_string_v<U>) this->pack_string(t);
        else if constexpr(impl::is_bool_v<U>) this->pack_bool(t);
        else if constexpr(std::is_enum_v<U>) this->pack_int(static_cast<std::underlying_type_t<U>>(t));
        else if constexpr(std::is_integral_v<U>) this->pack_int(t);
        else if constexpr(std::is_null_pointer_v<U>) this->pack_format(impl::format::NIL);
        else static_assert(impl::always_false_v<U>, "MsgPack::pack(): Unsupported type");
        return *this;
    }

    template<typename T>
    type& unpack(T& t) {
        using U = std::decay_t<T>;

        if constexpr(impl::is_array_v<U>) {
            size_t len = this->unpack_array();

            if constexpr(impl::is_vector_v<U>) t.reserve(len);

            for(size_t i = 0; i < len; ++i) {
                typename U::value_type v;
                this->unpack(v);
                if constexpr(impl::is_vector_v<U>) t.push_back(v);
                else t[i] = v;
            }
        }
        else if constexpr(impl::is_map_v<U>) {
            size_t len = this->unpack_map();

            for(size_t i = 0; i < len; ++i) {
                typename U::key_type k;
                typename U::mapped_type v;
                this->unpack(k);
                this->unpack(v);
                t[k] = v;
            }
        }
        else if constexpr(impl::is_string_v<U>) this->unpack_string(t);
        else if constexpr(impl::is_bool_v<U>) this->unpack_bool(t);
        else if constexpr(std::is_enum_v<U>) {
            std::underlying_type_t<U> u;
            this->unpack_int(u);
            t = static_cast<U>(u);
        }
        else if constexpr(std::is_integral_v<U>) this->unpack_int(t);
        else if constexpr(std::is_null_pointer_v<U>) {
            std::uint8_t f = this->unpack_format();
            assert(f == impl::format::NIL);
            t = nullptr;
        }
        else static_assert(impl::always_false_v<U>, "MsgPack::pack(): Unsupported type");

        return *this;
    }

    // Low Level Interface
    type& pack_map(std::size_t size) { return this->pack_aggregate(size, {impl::format::FIXMAP, impl::format::MAP16, impl::format::MAP32}); }
    type& pack_array(std::size_t size) { return this->pack_aggregate(size, {impl::format::FIXARRAY, impl::format::ARRAY16, impl::format::ARRAY32}); }
    inline size_t unpack_map() { return this->unpack_aggregate({impl::format::FIXMAP, impl::format::MAP16, impl::format::MAP32}); }
    inline size_t unpack_array() { return this->unpack_aggregate({impl::format::FIXARRAY, impl::format::ARRAY16, impl::format::ARRAY32}); }

private: // Packing
    inline void pack_bool(bool b) { this->pack_format(b ? impl::format::TRUE : impl::format::FALSE); }

    template<typename T>
    void pack_string(T&& t) {
        using U = std::decay_t<T>;
        const char* p = nullptr;
        size_t sz = 0;

        if constexpr(std::is_same_v<U, std::string> || std::is_same_v<U, std::string_view>) {
            p = t.data();
            sz = t.size();
        }
        else if constexpr(std::is_same_v<U, const char*>) {
            assert(t);
            p = t;
            sz = std::strlen(t);
        }
        else
            static_assert(impl::always_false_v<U>, "MsgPack::pack_string(): Unsupported string type");

        if(sz <= 31) {
            this->pack_format(impl::format::FIXSTR | static_cast<std::uint8_t>(sz));
            this->pack_raw(p, sz);
        }
        else if(sz <= std::numeric_limits<std::uint8_t>::max()) {
            this->pack_format(impl::format::STR8);
            this->pack_int(static_cast<std::uint8_t>(sz));
            this->pack_raw(p, sz);
        }
        else if(sz <= std::numeric_limits<std::uint16_t>::max()) {
            this->pack_format(impl::format::STR16);
            this->pack_int(static_cast<std::uint16_t>(sz));
            this->pack_raw(p, sz);
        }
        else if(sz <= std::numeric_limits<std::uint32_t>::max()) {
            this->pack_format(impl::format::STR32);
            this->pack_int(static_cast<std::uint32_t>(sz));
            this->pack_raw(p, sz);
        }
        else
            impl::msgpack_except("MsgPack::pack::string(): Unsupported string size");
    }

    template<typename T, typename = std::enable_if_t<std::is_integral_v<std::decay_t<T>>>>
    void pack_int(T t) { 
        if constexpr(sizeof(T) > sizeof(std::uint8_t)) t = impl::swap_bigendian(t);

        if constexpr(sizeof(T) == sizeof(std::uint8_t)) this->pack_format(std::is_signed_v<T> ? impl::format::INT8 : impl::format::UINT8);
        else if constexpr(sizeof(T) == sizeof(std::uint16_t)) this->pack_format(std::is_signed_v<T> ? impl::format::INT16 : impl::format::UINT16);
        else if constexpr(sizeof(T) == sizeof(std::uint32_t)) this->pack_format(std::is_signed_v<T> ? impl::format::INT32 : impl::format::UINT32);
        else if constexpr(sizeof(T) == sizeof(std::uint64_t)) this->pack_format(std::is_signed_v<T> ? impl::format::INT64 : impl::format::UINT64);
        else static_assert(impl::always_false_v<T>, "MsgPack::pack_int(): Unsupported integer type");

        this->pack_raw(reinterpret_cast<const value_type*>(&t), sizeof(T));
    }

    template<typename T>
    inline void pack_length(T len) { 
        len = impl::swap_bigendian(len);
        this->pack_raw(reinterpret_cast<value_type*>(&len), sizeof(len));
    }

    inline void pack_format(uint8_t f) { this->pack_raw(reinterpret_cast<const value_type*>(&f), sizeof(uint8_t)); }

    void pack_raw(const value_type* p, std::size_t size) {
        assert(p && size);
        this->buffer.get().reserve(this->buffer.get().size() + size);
        std::copy_n(p, size, std::back_inserter(this->buffer.get()));
    }

    inline type& pack_aggregate(std::size_t size, const std::array<std::uint8_t, 3>& formats) {
        if(size <= 0xF) {
            this->pack_format(formats[0] | static_cast<std::uint8_t>(size));
        }
        else if(size <= std::numeric_limits<uint16_t>::max()) {
            this->pack_format(formats[1]);
            this->pack_length(static_cast<std::uint16_t>(size));
        }
        else if(size <= std::numeric_limits<uint32_t>::max()) {
            this->pack_format(formats[2]);
            this->pack_length(static_cast<std::uint32_t>(size));
        }
        else
            impl::msgpack_except("MsgPack::new_aggregate(): Unsupported Size");

        return *this;
    }

private: // Unpacking
    size_t unpack_aggregate(const std::array<std::uint8_t, 3>& formats) {
        uint8_t f = this->unpack_format();
        size_t len = 0;

        if((f & 0xF0) == formats[0]) { len = f & 0x0F; }
        else if(f == formats[1]) len = this->unpack_length<std::uint16_t>();
        else if(f == formats[2]) len = this->unpack_length<std::uint32_t>(); 
        else impl::msgpack_except("MsgPack::unpack_aggregate(): Invalid Format");

        return len;
    }

    template<typename T>
    inline T unpack_length() { 
        T len = 0;
        this->unpack_raw(reinterpret_cast<value_type*>(&len), sizeof(T));
        return impl::swap_bigendian(len);
    }

    template<typename T>
    void unpack_string(T& t) {
        uint8_t f = this->unpack_format();
        size_t len = 0;

        if((f & 0xE0) == impl::format::FIXSTR) { len = f & 0x1F; }
        else if(f == impl::format::STR8) { std::uint8_t l; this->unpack_int(l); len = l; }
        else if(f == impl::format::STR16) { std::uint16_t l; this->unpack_int(l); len = l; }
        else if(f == impl::format::STR32) { std::uint32_t l; this->unpack_int(l); len = l; }
        else impl::msgpack_except("MsgPack::unpack_string(): Invalid Format");

        if constexpr(std::is_same_v<T, std::string>) {
            t.resize(len);
            this->unpack_raw(t.data(), len);
        }
        else if constexpr(std::is_same_v<T, std::string_view>) {
            t = std::string_view{this->buffer.get().data() + this->pos, len};
            this->pos += len;
        }
        else if constexpr(std::is_same_v<T, char const*>) this->unpack_raw(&t, len);
        else static_assert(impl::always_false_v<T>, "MsgPack::unpack_string(): Invalid type");
    }
    
    template<typename T, typename = std::enable_if_t<std::is_integral_v<std::decay_t<T>>>>
    void unpack_int(T& t) {
        using U = std::decay_t<T>;
        uint8_t f = this->unpack_format();

        switch(f) {
            case impl::format::INT8: assert((std::is_same_v<U, std::int8_t>)); break;
            case impl::format::INT16: assert((std::is_same_v<U, std::int16_t>)); break;
            case impl::format::INT32: assert((std::is_same_v<U, std::int32_t>)); break;
            case impl::format::INT64: assert((std::is_same_v<U, std::int64_t>)); break;
            case impl::format::UINT8: assert((std::is_same_v<U, std::uint8_t>)); break;
            case impl::format::UINT16: assert((std::is_same_v<U, std::uint16_t>)); break;
            case impl::format::UINT32: assert((std::is_same_v<U, std::uint32_t>)); break;
            case impl::format::UINT64: assert((std::is_same_v<U, std::uint64_t>)); break;
            default: impl::msgpack_except("MsgPack::unpack_int(): Invalid integer format"); break;
        }

        this->unpack_raw(reinterpret_cast<value_type*>(&t), sizeof(U));
        if constexpr(sizeof(U) > sizeof(std::uint8_t)) t = impl::swap_bigendian(t);
    }

    void unpack_bool(bool& b) {
        std::uint8_t f = this->unpack_format();
        if(f == impl::format::TRUE) b = true;
        else if(f == impl::format::FALSE) b = false;
        else impl::msgpack_except("MsgPack::unpack_bool(): Invalid Format");
    }

    uint8_t unpack_format() {
        std::uint8_t f;
        this->unpack_raw(reinterpret_cast<value_type*>(&f), sizeof(uint8_t));
        return f;
    }

    void unpack_raw(value_type* p, size_t size) {
        assert(p && size);
        auto endpos = this->pos + size;
        if(endpos > this->buffer.get().size()) impl::msgpack_except("MsgPack::unpack_raw(): Reached EOB");
        std::copy_n(this->buffer.get().begin() + this->pos, size, p);
        this->pos = endpos;
    }

public:
    std::reference_wrapper<Container> buffer;
    size_t pos{};
};

using MsgPack = BasicMsgPack<std::vector<char>>;

template<typename MsgPackType = MsgPack, typename VisitorType>
void visit(typename MsgPackType::container_type& c, VisitorType&& visitor) {
    MsgPackType mp{c};

    while(mp.pos < c.size()) {
        if(!impl::visit(mp, std::move(visitor))) break;
    }
}

template<typename T, typename MsgPackType = MsgPack>
typename MsgPackType::container_type pack(T&& t) {
    typename MsgPackType::container_type c;
    MsgPackType mp{c};
    mp.pack(t);
    return c;
}

template<typename T, typename MsgPackType = MsgPack>
void unpack(typename MsgPackType::container_type& buffer, T& t) {
    MsgPackType mp{buffer};
    mp.unpack(t);
}

} // namespace msgpack
