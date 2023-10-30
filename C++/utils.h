#pragma once

template<class... Ts> struct Overload : Ts... { using Ts::operator()...; };

#if __cplusplus >= 202002L
template<class... Ts> Overload(Ts...) -> Overload<Ts...>; 
#endif // __cplusplus >= 202002L

template<typename> constexpr bool always_false_v = false;
