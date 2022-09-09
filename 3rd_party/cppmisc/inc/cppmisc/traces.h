#pragma once

#include <cppmisc/misc.h>
#include <unistd.h>
#include <cassert>
#include <stdio.h>
#include "formatting.h"
#include "json.h"

namespace traces
{
    extern bool __enable_dbg;
    extern bool __enable_warn;
    extern bool __enable_err;
    extern bool __enable_info;

    void init(Json::Value const& cfg);
    bool enable_dbg();
    bool enable_warn();
    bool enable_err();
    bool enable_info();

    inline void write_stdout(std::string const& s)
    {
        int ans = write(1, s.c_str(), s.size());
        assert(ans >= 0);
        unused(ans);
    }

    inline void write_stderr(std::string const& s)
    {
        int ans = write(2, s.c_str(), s.size());
        assert(ans >= 0);
        unused(ans);
    }
}

template <class ... Args>
inline void info_msg(Args const& ... args)
{
    if (!traces::__enable_info)
        return;
    auto const& s = format("[inf] ", args..., "\n");
    traces::write_stdout(s);
}

template <class ... Args>
inline void warn_msg(Args const& ... args)
{
    if (!traces::__enable_warn)
        return;
    auto const& s = format("[wrn] ", args..., "\n");
    traces::write_stderr(s);
}

template <class ... Args>
inline void err_msg(Args const& ... args)
{
    if (!traces::__enable_err)
        return;
    auto const& s = format("[err] ", args..., "\n");
    traces::write_stderr(s);
}

template <class ... Args>
inline void dbg_msg(Args const& ... args)
{
    if (!traces::__enable_dbg)
        return;
    auto const& s = format("[dbg] ", args..., "\n");
    traces::write_stdout(s);
}

template <class ... Args>
inline void print_msg(Args const& ... args)
{
    auto const& s = format(args..., "\n");
    traces::write_stdout(s);
}
