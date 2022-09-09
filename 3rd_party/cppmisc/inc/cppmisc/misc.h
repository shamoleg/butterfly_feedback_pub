#pragma once

#include <cppmisc/throws.h>
#include <algorithm>
#include <initializer_list>


template <class T>
inline void unused(T t) { (void)t; }

template <typename T>
inline bool one_of(T const& value, std::initializer_list<T> const& values)
{
    auto i = std::find(values.begin(), values.end(), value);
    return i != values.end();
}

inline bool one_of(std::string const& value, std::initializer_list<char const*> const& values)
{
    for (auto s : values)
    {
        if (value == s)
            return true;
    }
    return false;
}

inline bool tobool(std::string const& s)
{
    std::string lower;
    lower.resize(s.length());
    std::transform(s.begin(), s.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
    if (lower == "true")
        return true;
    if (lower == "false")
        return false;
    int val = std::stoi(lower);
    return val != 0;
}
