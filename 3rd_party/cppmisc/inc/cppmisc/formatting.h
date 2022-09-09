#pragma once

#include <ostream>
#include <sstream>


inline void __format(std::ostream& s)
{
}

template <typename Arg, typename ... Args>
static void __format(std::ostream& s, Arg const& arg, Args const& ... args)
{
    s << arg;
    __format(s, args...);
}

template <typename ... Args>
static std::string format(Args const& ... args)
{
    std::stringstream s;
    __format(s, args...);
    return s.str();
}
