// Copyright (c)  AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "accelbyte/common/DateTime.h"
#include "accelbyte/common/Map.h"
#include "accelbyte/common/Optional.h"
#include "accelbyte/common/String.h"
#include "accelbyte/common/Vector.h"
#include "accelbyte/http/ModelObject.h"
#include "accelbyte/memory/memory.h"
#include "accelbyte/utils/JsonObjectString.h"
#include "codegen_common_export.h"

#include <functional>
#include <iostream>
#include <json/value.h>
#include <type_traits>

namespace accelbyte {
namespace common {

class SerializerFactory;

class Serializer {
public:
    ACCELBYTE_CODEGEN_COMMON_API Serializer() = default;
    ACCELBYTE_CODEGEN_COMMON_API explicit Serializer(const std::shared_ptr<SerializerFactory>& factory);
    ACCELBYTE_CODEGEN_COMMON_API virtual ~Serializer() = default;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const String& key, const accelbyte::http::ModelObject& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const accelbyte::http::ModelObject& value, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const utils::JsonObjectString& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const String& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const unsigned long& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const long& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const String& key, const accelbyte::DateTime& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const String& key, const utils::JsonObjectString& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const String& key, const String& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const String& key, const unsigned long& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const String& key, const long& from, Json::Value& to) const;

    ACCELBYTE_CODEGEN_COMMON_API void append_value(const DateTime& from, Json::Value& to) const;

    template<typename T>
    auto append_value(const T& value, Json::Value& to) const ->
        typename std::enable_if<!std::is_base_of<accelbyte::http::ModelObject, T>::value, void>::type
    {
        to.append(value);
    }

    template<typename T>
    auto append_value(const String& key, const T& from, Json::Value& to) const ->
        typename std::enable_if<!std::is_base_of<accelbyte::http::ModelObject, T>::value, void>::type
    {
        to[key.c_str()] = from;
    }

    template<typename T>
    void append_value(const String& key, const Vector<T>& from, Json::Value& to) const
    {
        Json::Value v;
        v.resize(0);
        for (const auto& e : from) { append_value(e, v); }
        to[key.c_str()] = v;
    }

    template<typename T>
    void append_value(const String& key, const Optional<T>& from, Json::Value& to) const
    {
        if (from.has_value()) {
            append_value(key, from.value(), to);
        }
    }

    template<typename K, typename T>
    void append_value(const String& key, const Map<K, T>& from, Json::Value& to) const
    {
        for (const auto& val : from) {
            Json::Value v;
            append_value(val.first, val.second, v);
            to[key.c_str()] = v;
        }
    }

    template<typename T>
    void append_enum_value(const String& key, const T& from, Json::Value& to, String(*translator)(const T&)) const
    {
        append_value(key, translator(from),to);
    }

    template<typename T>
    void
    append_enum_value(const String& key, const Optional<Vector<T>>& from, Json::Value& to, String (*translator)(const T&)) const
    {
        if (from.has_value()) {
            append_enum_value(key, from.value(), to, translator);
        }
    }

    template<typename T>
    void
    append_enum_value(const String& key, const Optional<T>& from, Json::Value& to, String (*translator)(const T&)) const
    {
        if (from.has_value()) {
            append_enum_value(key, from.value(), to, translator);
        }
    }

    template<typename T>
    void append_enum_value(const String& key, const Vector<T>& from, Json::Value& to, String (*translator)(const T&)) const
    {
        Json::Value v;
        v.resize(0);
        for (const auto& e : from) { append_value(translator(e) ,v); }
        to[key.c_str()] = v;
    }

    template<typename K, typename T>
    void append_enum_value(const String& key, const Map<K, T>& from, Json::Value& to, String (*translator)(const T&)) const
    {
        for (const auto& val : from) {
            Json::Value v;
            append_enum_value(val.first, val.second, v, translator);
            to[key.c_str()] = v;
        }
    }

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, int& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, unsigned int& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, long& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, unsigned long& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, float& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, double& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, bool& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, String& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, utils::JsonObjectString& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, DateTime& to) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto extract_value(const Json::Value& from, accelbyte::http::ModelObject& to) const -> bool;

    template<typename T>
    auto extract_value(const Json::Value&, T&) const ->
        typename std::enable_if<!std::is_base_of<accelbyte::http::ModelObject, T>::value, bool>::type

    {
        return false;
    }

    template<typename T>
    auto extract_value(const Json::Value& from, Vector<T>& to) const -> bool
    {
        bool res = true;
        for (const auto& e : from) {
            T val;
            res &= extract_value(e, val);
            to.push_back(val);
        }
        return res;
    }

    template<typename T>
    auto extract_value(const Json::Value& from, Map<String, T>& to) const -> bool
    {
        bool res = true;
        for (const auto& e : from.getMemberNames()) {
            T val;
            res &= extract_value(from[e], val);
            to[e.c_str()] = val;
        }
        return res;
    }

    template<typename T>
    auto extract_value(const String& key, const Json::Value& from, T& to, String& message) const -> bool
    {
        if (from.isMember(key.c_str())) {
            bool res = extract_value(from[key.c_str()], to);
            return res;
        }
        message += "Missing " + key + " field.";
        return false;
    }

    template<typename T>
    auto extract_value(const String& key, const Json::Value& from, Optional<T>& to, String& message) const -> bool
    {
        if (from.isMember(key.c_str()) && !from[key.c_str()].isNull()) {
            to.emplace();
            return extract_value(key, from, to.value(), message);
        }
        return true;
    }

    template<typename T>
    auto extract_value(const String& key, const Json::Value& from, Vector<T>& to, String& message) const -> bool
    {
        if (from.isMember(key.c_str())) {
            auto toExtract = from[key.c_str()];
            bool res = true;
            for (const auto& e : toExtract) {
                T val;
                res &= extract_value(e, val);
                to.push_back(val);
            }
            return res;
        }
        message += "Missing " + key + " field.";
        return false;
    }

    template<typename K, typename T>
    auto extract_value(const String& key, const Json::Value& from, Map<K, T>& to, String& message) const -> bool
    {
        if (from.isMember(key.c_str())) {
            auto toExtract = from[key.c_str()];
            bool res = true;
            for (const auto& e : toExtract.getMemberNames()) {
                T val;
                res &= extract_value(toExtract[e], val);
                to[e.c_str()] = val;
            }
            if (!res) {}
            return res;
        }
        message += "Missing " + key + " field.";
        return false;
    }

    template<typename T>
    auto extract_enum_value(const Json::Value& from, Vector<T>& to, T (*translator)(const String&)) const -> bool
    {
        bool res = true;
        for (const auto& e : from) {
            T val;
            res &= extract_enum_value(e, val, translator);
            to.push_back(val);
        }
        return res;
    }

    template<typename T>
    auto extract_enum_value(const Json::Value& from, T& to, T (*translator)(const String&)) const -> bool
    {
        String val;
        bool res = extract_value(from, val);
        if (res) {
            to = translator(val);
        }
        return res;
    }

    template<typename T>
    auto extract_enum_value(const String& key, const Json::Value& from, T& to, T (*translator)(const String&), String& message) const
        -> bool
    {
        if (from.isMember(key.c_str())) {
            bool res = extract_enum_value(from[key.c_str()], to, translator);
            return res;
        }
        message += "Missing " + key + " field.";
        return false;
    }

    template<typename T>
    auto extract_enum_value(const String& key, const Json::Value& from, Vector<T>& to, T (*translator)(const String&), String& message) const
        -> bool
    {
        if (from.isMember(key.c_str())) {
            auto toExtract = from[key.c_str()];
            bool res = true;
            for (const auto& e : toExtract) {
                T val;
                res &= extract_enum_value(e, val, translator);
                to.push_back(val);
            }
            return res;
        }
        message += "Missing " + key + " field.";
        return false;
    }

    template<typename T>
    auto extract_enum_value(
        const String& key, const Json::Value& from, Optional<T>& to, T (*translator)(const String&), String& message) const -> bool
    {
        if (from.isMember(key.c_str()) && !from[key.c_str()].isNull()) {
            to.emplace();
            return extract_enum_value(from[key.c_str()], to.value(), translator);
        }
        message += "Missing " + key + " field.";
        return true;
    }

    template<typename T>
    auto extract_enum_value(
        const String& key, const Json::Value& from, Optional<Vector<T>>& to, T (*translator)(const String&), String& message) const -> bool
    {
        if (from.isMember(key.c_str()) && !from[key.c_str()].isNull()) {
            to.emplace();
            return extract_enum_value(key, from, to.value(), translator, message);
        }
        return true;
    }

    ACCELBYTE_CODEGEN_COMMON_API auto to_json(const http::ModelObject& from, String& to, String& messages) const -> bool;
    ACCELBYTE_CODEGEN_COMMON_API virtual auto to_json_object(const http::ModelObject& from, Json::Value& to, String& messages) const -> bool;

    ACCELBYTE_CODEGEN_COMMON_API auto from_json(const String& from, http::ModelObject& to, String& messages) const -> bool;
    ACCELBYTE_CODEGEN_COMMON_API virtual auto from_json_object(const Json::Value& from, http::ModelObject& to, String& messages) const -> bool;

protected:
    ACCELBYTE_CODEGEN_COMMON_API auto create_sub_model(const accelbyte::http::ModelObject& value, Json::Value& to, String& messages) const -> bool;
    ACCELBYTE_CODEGEN_COMMON_API auto append_sub_model(const String& key, const http::ModelObject& value, Json::Value& to, String& messages) const -> bool;

    template<class T>
    auto append_sub_model(const String& key, const T& from, Json::Value& to, String& messages) const -> bool
    {
        return append_sub_model(key, from, to, messages);
    }

    template<class T>
    auto append_sub_model(const String& key, const Optional<T>& from, Json::Value& to, String& messages) const -> bool
    {
        if (from.has_value()) {
            return append_sub_model(key, from.value(), to, messages);
        }
        return true;
    }

    ACCELBYTE_CODEGEN_COMMON_API auto extract_sub_model(const String& key, const Json::Value& from, http::ModelObject& to, String& messages) const -> bool;

    template<class T>
    auto extract_sub_model(const String& key, const Json::Value& from, Optional<T>& to, String& messages) const -> bool
    {
        if (from.isMember(key.c_str())) {
            to.emplace();
            return extract_sub_model(key, from, to.value(), messages);
        }
        return true;
    }

protected:
    std::shared_ptr<SerializerFactory> factory_;
};

} // namespace common
} // namespace accelbyte