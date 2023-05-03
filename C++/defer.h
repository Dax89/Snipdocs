#pragma once

template<typename Defer>
struct defer_guard
{
    defer_guard() = delete;
    defer_guard(Defer d_): d{d_} { }
    ~defer_guard() { this->d(); }

    Defer d;
};

template<typename Defer>
auto defer(Defer d) { return defer_guard<Defer>{d}; }
