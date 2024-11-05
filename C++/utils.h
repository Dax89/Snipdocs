#pragma once

#include <cctype>
#include <cstdint>
#include <string_view>
#include <memory>
#include <vector>

using Data = std::vector<uint8_t>;

template<typename T>
using Impl = const std::unique_ptr<T>;

template<class... Ts>
struct Overload : Ts... { using Ts::operator()...; };

#if __cplusplus < 202002L
template<class... Ts>
Overload(Ts...) -> Overload<Ts...>; 
#endif // __cplusplus >= 202002L

template<typename> constexpr bool always_false_v = false;

template<typename Function>
void split_each(std::string_view s, char sep, Function f) {
    size_t i = 0, start = 0;

    for( ; i < s.size(); i++) {
        if(s[i] != sep) continue;
        if(i > start) f(s.substr(start, i - start));
        start = i + 1;
    }

    if(start < s.size()) f(s.substr(start));
}

inline bool starts_with(std::string_view s, std::string_view w) {
    return s.find(w) == 0;
}
inline bool ends_with(std::string_view s, std::string_view w) {
    return s.size() >= w.size() &&
           s.compare(s.size() - w.size(), w.size(), w) == 0;
}
