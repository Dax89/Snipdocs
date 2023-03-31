#pragma once

#include <cstdio>
#include <utility>
#include <spdlog/spdlog.h>

#if defined(__GNUC__) // GCC, Clang, ICC
    #define intrinsic_trap()        __builtin_trap()
    #define intrinsic_unreachable() __builtin_unreachable()
    #define intrinsic_unlikely(x)   __builtin_expect(!!(x), 0)
#elif defined(_MSC_VER) // MSVC
    #define intrinsic_trap()        __debugbreak()
    #define intrinsic_unreachable() __assume(false)
    #define intrinsic_unlikely(x)   (!!(x))
#else
    #define intrinsic_trap()        std::abort()
    #define intrinsic_unreachable() std::abort()
    #define intrinsic_unlikely(x)   (!!(x))
#endif

namespace impl {

[[noreturn]] inline void trap() {
    std::fflush(nullptr);
    intrinsic_trap();
}

[[noreturn]] inline void abort() {
    std::fflush(nullptr);
    std::abort();
}

[[noreturn]] inline void unreachable() {
#if defined(NDEBUG) // Release
    std::fflush(nullptr);
    intrinsic_unreachable();
#else
    impl::trap();
#endif
}
    
} // namespace impl

#define assume(...) \
    do { \
        if(intrinsic_unlikely(!(__VA_ARGS__))) { \
            SPDLOG_ERROR("Assume condition failed: '{}'", #__VA_ARGS__); \
            ::impl::trap(); \
        } \
    } while(false)

#define unreachable \
    do { \
        SPDLOG_ERROR("Unreachable code detected"); \
        ::impl::unreachable(); \
    } while(false)

#define except(...) \
    do { \
        SPDLOG_ERROR(__VA_ARGS__); \
        ::impl::abort(); \
    } while(false)
