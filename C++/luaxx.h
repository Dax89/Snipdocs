#pragma once

#include <lua.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace redasm::lua::luaxx {

using Nil = std::nullptr_t;
using Bool = bool;
using Integer = size_t;
using Number = double;
using String = std::string_view;
enum class List {};
enum class Table {};

struct Function;

using Type = std::variant<Nil, Bool, Integer, Number, String, lua_CFunction,
                          Function, List, Table>;
using Pair = std::pair<std::string_view, Type>;

struct Function {
    explicit Function(lua_CFunction f, std::vector<Type>&& up = {})
        : func{f}, upvalues(std::move(up)) {}

    lua_CFunction func;
    std::vector<Type> upvalues;
};

inline constexpr int KEY = -2;
inline constexpr int VALUE = -1;

inline int to_index(Table x) { return static_cast<int>(x); }
inline int to_index(List x) { return static_cast<int>(x); }

template<typename Function>
bool table_each(lua_State* l, int idx, Function f) {
    if(int t = lua_type(l, idx); t != LUA_TTABLE) {
        luaL_error(l, "table_each: Cannot iterate type '%s'",
                   lua_typename(l, t));

        return false;
    }

    if(idx < 0)
        idx -= 1;

    size_t i = 0;
    bool ok = true;

    lua_pushnil(l);
    int hasmore;

    while(ok && (hasmore = lua_next(l, idx))) {
        ok = f(i++);
        lua_pop(l, 1); // Pop Value, Keep key
    }

    if(hasmore)
        lua_pop(l, 1); // Pop key

    return ok;
}

lua_State* init();
inline void deinit(lua_State* l) { lua_close(l); }
bool table_islist(lua_State* l, int idx);
void push_type(lua_State* l, const Type& t);
void new_metatable(lua_State* l, const Type& t);
void set_metatable(lua_State* l, int index, const Type& t);
void new_object(lua_State* l, std::initializer_list<Pair> fields = {});
void new_list(lua_State* l, std::initializer_list<Type> items = {});
void set_field(lua_State* l, int index, std::string_view k, const Type& v);

inline std::optional<std::string> eval(lua_State* l, std::string_view s) {
    if(luaL_dostring(l, s.data()) != LUA_OK)
        return lua_tostring(l, -1);
    return std::nullopt;
}

namespace impl {

template<typename>
inline constexpr bool AlwaysFalseV = false; // NOLINT

template<size_t Index, typename TupleType>
void parse_args_impl(lua_State* l, TupleType& tt) {
    if constexpr(Index < std::tuple_size_v<TupleType>) {
        using U = std::tuple_element_t<Index, TupleType>;
        int t = lua_type(l, Index + 1);

        if constexpr(std::is_same_v<U, Nil>) {
            if(t == LUA_TNIL)
                std::get<Index>(tt) = nullptr;
            else
                luaL_error(l, "Expected 'nil', got '%s'", lua_typename(l, t));
        }
        else if constexpr(std::is_same_v<U, Bool>) {
            if(t == LUA_TBOOLEAN)
                std::get<Index>(tt) = !!lua_toboolean(l, Index + 1);
            else
                luaL_error(l, "Expected 'bool', got '%s'", lua_typename(l, t));
        }
        else if constexpr(std::is_floating_point_v<U>) {
            if(t == LUA_TNUMBER)
                std::get<Index>(tt) = lua_tonumber(l, Index + 1);
            else
                luaL_error(l, "Expected 'number', got '%s'",
                           lua_typename(l, t));
        }
        else if constexpr(std::is_integral_v<U>) {
            if(t == LUA_TNUMBER)
                std::get<Index>(tt) = lua_tointeger(l, Index + 1);
            else
                luaL_error(l, "Expected 'integer', got '%s'",
                           lua_typename(l, t));
        }
        else if constexpr(std::is_same_v<U, std::string_view> ||
                          std::is_same_v<U, std::string>) {
            if(t == LUA_TSTRING)
                std::get<Index>(tt) = lua_tostring(l, Index + 1);
            else
                luaL_error(l, "Expected 'string', got '%s'",
                           lua_typename(l, t));
        }
        else if constexpr(std::is_same_v<U, List>) {
            if(luaxx::table_islist(l, Index + 1)) {
                std::get<Index>(tt) = static_cast<Table>(Index + 1);
            }
            else
                luaL_error(l, "Expected 'list', got '%s'", lua_typename(l, t));
        }
        else if constexpr(std::is_same_v<U, Table>) {
            if(t == LUA_TTABLE && !luaxx::table_islist(l, Index + 1)) {
                std::get<Index>(tt) = static_cast<Table>(Index + 1);
            }
            else
                luaL_error(l, "Expected 'object', got '%s'",
                           lua_typename(l, t));
        }
        else
            static_assert(AlwaysFalseV<U>);

        impl::parse_args_impl<Index + 1>(l, tt);
    }
}

} // namespace impl

template<typename... Args>
auto parse_args(lua_State* l) {
    std::tuple<Args...> args;
    int nargs = lua_gettop(l);

    if(nargs != sizeof...(Args)) {
        luaL_error(l, "Expected %d argument(s), got %d", sizeof...(Args),
                   nargs);
    }
    else
        impl::parse_args_impl<0>(l, args);

    return args;
}

} // namespace redasm::lua::luaxx
