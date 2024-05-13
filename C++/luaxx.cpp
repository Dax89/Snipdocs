#include "luaxx.h"

namespace redasm::lua::luaxx {

namespace {

template<class... Ts>
struct Overload: Ts... {
    using Ts::operator()...;
};

#if __cplusplus < 202002L
template<class... Ts>
Overload(Ts...) -> Overload<Ts...>;
#endif // __cplusplus >= 202002L

} // namespace

bool table_islist(lua_State* l, int idx) {
    if(lua_type(l, idx) != LUA_TTABLE)
        return false;

    lua_Integer c = 0;

    bool islist = luaxx::table_each(l, idx, [&](int) {
        if(lua_type(l, -2) == LUA_TNUMBER) {
            c += 1;
            return true;
        }

        return false;
    });

    return islist && c > 0;
}

void push_type(lua_State* l, const Type& t) {
    const auto VISITOR = luaxx::Overload{
        [l](Nil) { lua_pushnil(l); },
        [l](Bool x) { lua_pushboolean(l, x); },
        [l](Integer x) { lua_pushinteger(l, x); },
        [l](Number x) { lua_pushnumber(l, x); },
        [l](String x) { lua_pushlstring(l, x.data(), x.size()); },
        [l](lua_CFunction x) { lua_pushcfunction(l, x); },
        [l](const Function& x) {
            for(const Type& up : x.upvalues)
                luaxx::push_type(l, up);
            lua_pushcclosure(l, x.func, x.upvalues.size());
        },
        [l](List x) { lua_pushvalue(l, static_cast<int>(x)); },
        [l](Table x) { lua_pushvalue(l, static_cast<int>(x)); },
    };

    std::visit(VISITOR, t);
}

lua_State* init() {
    lua_State* l = luaL_newstate();
    if(l)
        luaL_openlibs(l);
    else
        return nullptr;
    return l;
}

void new_object(lua_State* l, std::initializer_list<Pair> fields) {
    lua_createtable(l, 0, fields.size());

    for(const auto& [key, val] : fields) {
        luaxx::push_type(l, val);
        lua_setfield(l, -2, key.data());
    }
}

void new_list(lua_State* l, std::initializer_list<Type> items) {
    lua_createtable(l, items.size(), 0);
    size_t i = 0;

    for(const auto& val : items) {
        luaxx::push_type(l, val);
        lua_rawseti(l, -2, i++);
    }
}

void new_metatable(lua_State* l, const Type& t) {
    lua_createtable(l, 0, 1);
    luaxx::push_type(l, t);
    lua_setfield(l, -2, "__index");
}

void set_metatable(lua_State* l, int index, const Type& t) {
    if(index < 0)
        index--;

    luaxx::new_metatable(l, t);
    lua_setmetatable(l, index);
}

void set_field(lua_State* l, int index, std::string_view k, const Type& v) {
    if(index < 0)
        index--;

    luaxx::push_type(l, v);
    lua_setfield(l, index, k.data());
}

} // namespace redasm::lua::luaxx
