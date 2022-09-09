#pragma once
#include <jsoncpp/json/json.h>
#include "throws.h"


inline void json_parse(Json::Value const& json, double& value)
{
    value = json.asDouble();
}

inline void json_parse(Json::Value const& json, float& value)
{
    value = json.asFloat();
}

inline void json_parse(Json::Value const& json, std::string& value)
{
    value = json.asCString();
}

inline void json_parse(Json::Value const& json, int& value)
{
    value = json.asInt();
}

inline void json_parse(Json::Value const& json, int64_t& value)
{
    value = json.asInt64();
}

template <typename T>
inline void json_parse(Json::Value const& json, std::vector<T>& arr)
{
    const int n = json.size();
    arr.resize(n);

    for (int i = 0; i < n; ++ i)
        json_parse(json[i], arr[i]);
}

template <typename T, size_t N>
inline void json_parse(Json::Value const& json, std::array<T, N>& arr)
{
    const int n = json.size();
    if (n != N)
        throw_runtime_error("Can't parse std::array, incorrect size: expect ", N, " but actual is ", n);

    for (int i = 0; i < int(N); ++ i)
        json_parse(json[i], arr[i]);
}

template <typename T=Json::Value>
inline T json_get(Json::Value const& json, char const* name)
{
    T result;

    if (!json.isMember(name))
        throw_runtime_error("Json ", json, " does not contain entry ", name);

    json_parse(json[name], result);
    return result;
}

template <>
inline Json::Value json_get(Json::Value const& json, char const* name)
{
    if (!json.isMember(name))
        throw_runtime_error("Json ", json, " does not contain entry ", name);

    return json[name];
}

template <typename T>
inline void json_get(Json::Value const& json, char const* name, T& value)
{
    if (!json.isMember(name))
        throw_runtime_error("Json ", json, " does not contain entry ", name);

    json_parse(json[name], value);
}

template <typename T>
inline void json_get(Json::Value const& json, int idx, T& value)
{
    json_parse(json[idx], value);
}

template <typename T>
inline T json_get(Json::Value const& json, int idx)
{
    T result;
    json_parse(json[idx], result);
    return result;
}

Json::Value json_load(std::string const& filepath);
Json::Value json_parse(char const* str);
inline Json::Value json_parse(std::string const& s) { return json_parse(s.c_str()); }

inline bool json_has(Json::Value const& v, std::string const& name)
{
    if (!v.isMember(name))
        return false;
    return true;
}
