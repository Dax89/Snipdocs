// spdlog::set_pattern("%s|%!:%# - %v");

#pragma once

#include <cstdio>
#include <utility>
#include <spdlog/spdlog.h>

namespace impl {

[[noreturn]] inline void abort() {
    std::fflush(nullptr);
    std::abort();
}
    
} // namespace impl

#define assume(...) \
    do { \
        if(!(__VA_ARGS__)) { \
            SPDLOG_ERROR("Assume condition failed"); \
            ::impl::abort(); \
        } \
    } while(false)

#define unreachable \
    do { \
        SPDLOG_ERROR("Unreachable code detected"); \
        ::impl::abort(); \
    } while(false)

#define except(...) \
    do { \
        SPDLOG_ERROR(__VA_ARGS__); \
        ::impl::abort(); \
    } while(false)
