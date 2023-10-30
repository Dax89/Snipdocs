#pragma once

#include <cctype>
#include <string_view>

template<class... Ts> struct Overload : Ts... { using Ts::operator()...; };

#if __cplusplus < 202002L
template<class... Ts> Overload(Ts...) -> Overload<Ts...>; 
#endif // __cplusplus >= 202002L

template<typename> constexpr bool always_false_v = false;

template<typename Function>
void split_each(std::string_view s, char sep, Function f) {
    size_t i = 0, start = 0;

    for( ; i < s.size(); i++) {
        if(std::isspace(s[i])) continue;
        if(s[i] != sep) continue;
        if(i > start) f(s.substr(start, i - start));
        start = i + 1;
    }

    while(start < s.size() && std::isspace(s[start])) start++;
    if(start < s.size()) f(s.substr(start));
}
