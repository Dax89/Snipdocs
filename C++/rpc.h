#pragma once

#include <QString>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <redasm/redasm.h>

namespace rpc {

namespace impl {

template<typename T>
void push_param(rapidjson::Document& req, rapidjson::Value& params, T&& arg) {
    using U = std::decay_t<T>;

    if constexpr(std::is_same_v<U, QString>) {
        params.PushBack(
            rapidjson::Value{qUtf8Printable(arg), req.GetAllocator()}.Move(),
            req.GetAllocator());
    }
    else if constexpr(std::is_same_v<U, rapidjson::Document>) {
        // Don't steal the ownership of the passed Document, but copy it
        params.PushBack(rapidjson::Value{arg, req.GetAllocator()},
                        req.GetAllocator());
    }
    else
        params.PushBack(arg, req.GetAllocator());
}

} // namespace impl

template<typename... Args>
rapidjson::Document call(const QString& method, Args&&... args) {
    rapidjson::Document req(rapidjson::kObjectType);
    req.AddMember("method",
                  rapidjson::Value{qUtf8Printable(method), req.GetAllocator()},
                  req.GetAllocator());

    rapidjson::Value params{rapidjson::kArrayType};
    (impl::push_param(req, params, args), ...);
    req.AddMember("params", params, req.GetAllocator());

    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w{sb};
    req.Accept(w);

    rapidjson::Document recv;
    recv.Parse(redasm::call_raw(sb.GetString()).c_str());

    if(recv.HasMember("error"))
        qFatal("%s", recv["error"]["message"].GetString());

    recv.Swap(recv["result"]);
    return recv;
}

} // namespace rpc
