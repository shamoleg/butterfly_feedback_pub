#pragma once
#include <stdexcept>
#include "formatting.h"


template <typename Exception, typename ... Args>
[[ noreturn ]] inline void throw_exception(Args const& ... args)
{
    std::string msg = format(args...);
    throw Exception(msg);
}

template <typename ... Args>
[[ noreturn ]] inline void throw_runtime_error(Args const& ... args)
{
    throw_exception<std::runtime_error>(args...);
}

template <typename ... Args>
[[ noreturn ]] inline void throw_invalid_argument(Args const& ... args)
{
    throw_exception<std::invalid_argument>(args...);
}
