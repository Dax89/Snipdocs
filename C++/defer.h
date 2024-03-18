#pragma once

#include <utility>

namespace defer_impl {

template<typename F>
class Defer {
public:
    Defer() = delete;
    Defer(const Defer&) = delete;
    Defer& operator=(const Defer&) = delete;
    explicit Defer(F f): m_f{std::move(f)} {}

    Defer(Defer&& rhs) noexcept: m_f{std::move(rhs.m_f)}, m_a{rhs.m_a} {
        rhs.m_a = false;
    }

    ~Defer() {
        if(m_a)
            m_f();
    }

private:
    F m_f;
    bool m_a{true};
};

enum class DeferTag {};

template<typename F>
Defer<F> operator+(DeferTag, F&& f) {
    return Defer<F>{std::forward<F>(f)};
}

} // namespace defer_impl

#define _defer_concatenate_impl(s1, s2) s1##s2
#define _defer_concatenate(s1, s2) _defer_concatenate_impl(s1, s2)

#ifdef __COUNTER__
    #define _defer_anonymous_variable(str) _defer_concatenate(str, __COUNTER__)
#else
    #define _defer_anonymous_variable(str) _defer_concatenate(str, __LINE__)
#endif

#define defer                                                                  \
    auto _defer_anonymous_variable(SCOPE_EXIT_STATE) =                         \
        ::defer_impl::DeferTag{} + [&]() // NOLINT
