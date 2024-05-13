#pragma once

#include <cstdint>
#include <type_traits>
#include <tuple>
#include <string>
#include <string_view>
#include <fmt/core.h>
#include <rapidjson/document.h>

#define RPC_CHECK_ERROR(err)                                                   \
    if(rpc::is_error(err))                                                     \
        return std::move(err);

namespace rpc {

[[nodiscard]] bool is_error(const rapidjson::Value& obj);
[[nodiscard]] rapidjson::Document response(const rapidjson::Document& result);

template<typename... Args>
[[nodiscard]] rapidjson::Document error(Args&&... args) {
    rapidjson::Document r{rapidjson::kObjectType};
    rapidjson::Value err{rapidjson::kObjectType};

    err.AddMember("message",
                  rapidjson::Value{fmt::format(std::forward<Args>(args)...),
                                   r.GetAllocator()}
                      .Move(),
                  r.GetAllocator());

    r.AddMember("error", err.Move(), r.GetAllocator());
    return r;
}

// clang-format off
enum class Arg { 
    BOOL = 0, 
    UINT, INT, FLOAT, DOUBLE, 
    STR, STRV, 
    ARR, OBJ
};

template<Arg arg> struct ArgType {};
template<> struct ArgType<Arg::BOOL>   { using Type = bool; };
template<> struct ArgType<Arg::UINT>   { using Type = size_t; };
template<> struct ArgType<Arg::INT>    { using Type = std::make_signed_t<size_t>; };
template<> struct ArgType<Arg::FLOAT>  { using Type = float; };
template<> struct ArgType<Arg::DOUBLE> { using Type = double; };
template<> struct ArgType<Arg::STR>    { using Type = std::string; };
template<> struct ArgType<Arg::STRV>   { using Type = std::string_view; };
template<> struct ArgType<Arg::ARR>    { using Type = const rapidjson::Value*; };
template<> struct ArgType<Arg::OBJ>    { using Type = const rapidjson::Value*; };

template<Arg A> using ArgTypeT = typename ArgType<A>::Type;
// clang-format on

namespace impl {

template<size_t Index, typename TupleType, Arg... Args>
void check_params(TupleType& t, const rapidjson::Value& params) {
    constexpr std::tuple<decltype(Args)...> ARGS(Args...);

    if constexpr(Index < sizeof...(Args)) {
        constexpr size_t T_INDEX = Index + 1;
        constexpr Arg ARG = std::get<Index>(ARGS);
        const rapidjson::Value& p = params[Index];

        if constexpr(ARG == Arg::BOOL) {
            if(p.IsBool())
                std::get<T_INDEX>(t) = p.GetBool();
            else {
                std::get<0>(t) =
                    rpc::error("Argument #{}: Expected 'BOOL', got '{}'",
                               T_INDEX, json::type_name(p));
            }
        }
        else if constexpr(ARG == Arg::INT) {
            if(p.IsInt64())
                std::get<T_INDEX>(t) = p.GetInt64();
            else {
                std::get<0>(t) =
                    rpc::error("Argument #{}: Expected 'INT', got '{}'",
                               T_INDEX, json::type_name(p));
            }
        }
        else if constexpr(ARG == Arg::UINT) {
            if(p.IsUint64())
                std::get<T_INDEX>(t) = p.GetUint64();
            else {
                std::get<0>(t) =
                    rpc::error("Argument #{}: Expected 'UINT', got '{}'",
                               T_INDEX, json::type_name(p));
            }
        }
        else if constexpr(ARG == Arg::FLOAT) {
            if(p.IsFloat())
                std::get<T_INDEX>(t) = p.GetFloat();
            else {
                std::get<0>(t) =
                    rpc::error("Argument #{}: Expected 'FLOAT', got '{}'",
                               T_INDEX, json::type_name(p));
            }
        }
        else if constexpr(ARG == Arg::DOUBLE) {
            if(p.IsDouble())
                std::get<T_INDEX>(t) = p.GetFloat();
            else {
                std::get<0>(t) =
                    rpc::error("Argument #{}: Expected 'DOUBLE', got '{}'",
                               T_INDEX, json::type_name(p));
            }
        }
        else if constexpr(ARG == Arg::STR || ARG == Arg::STRV) {
            if(p.IsString()) {
                std::get<T_INDEX>(t) = p.GetString();
            }
            else {
                std::get<0>(t) =
                    rpc::error("Argument #{}: Expected 'STR', got '{}'",
                               T_INDEX, json::type_name(p));
            }
        }
        else if constexpr(ARG == Arg::ARR) {
            if(p.IsArray()) {
                std::get<T_INDEX>(t) = &p;
            }
            else {
                std::get<0>(t) =
                    rpc::error("Argument #{}: Expected 'ARR', got '{}'",
                               T_INDEX, json::type_name(p));
            }
        }
        else if constexpr(ARG == Arg::OBJ) {
            if(p.IsObject()) {
                std::get<T_INDEX>(t) = &p;
            }
            else {
                std::get<0>(t) =
                    rpc::error("Argument #{}: Expected 'OBJ', got '{}'",
                               T_INDEX, json::type_name(p));
            }
        }
        else
            static_assert(utils::AlwaysFalseV<TupleType>);

        if(!rpc::is_error(std::get<0>(t)))
            impl::check_params<Index + 1, TupleType, Args...>(t, params);
    }
}

} // namespace impl

template<Arg... Args>
auto check_params(const rapidjson::Value& req) {
    using Result = std::tuple<rapidjson::Document, ArgTypeT<Args>...>;
    constexpr size_t N_ARGS = sizeof...(Args);

    const rapidjson::Value& params = req["params"];
    assume(params.IsArray());

    Result res;

    if(N_ARGS != params.Size()) {
        std::get<0>(res) = rpc::error("Expected {} argument(s), got {}", N_ARGS,
                                      params.Size());
    }
    else
        impl::check_params<0, Result, Args...>(res, params);

    return res;
}

} // namespace rpc
