#pragma once

#include utility
#include <tuple>

namespace impl {

template<typename T, size_t Index>
T get_sequence_value() {
    if constexpr(std::is_same_v<T, bool>) return true;
    else if constexpr(std::is_integral_v<T>) return 42;
    else if constexpr(std::is_same_v<T, std::string>) return "thestring";
    ...
}

template<typename... Args, size_t... Sequence>
auto tuple_sequence(std::index_sequence<Sequence...>) {
    return std::make_tuple(impl::get_sequence_value<Args, Sequence + 1>()...);
}

} // namespace impl

template<typename ...Args>
auto tuple_sequence() { return impl::tuple_sequence<Args...>(std::index_sequence_for<Args...>{}); } 
