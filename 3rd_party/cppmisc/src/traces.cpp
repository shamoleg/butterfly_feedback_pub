#include <cppmisc/traces.h>


namespace traces
{
    bool __enable_dbg = false;
    bool __enable_warn = true;
    bool __enable_err = true;
    bool __enable_info = true;

    void init(Json::Value const& cfg)
    {
        bool enable_dbg = false;
        bool enable_warn = false;
        bool enable_err = false;
        bool enable_info = false;
        auto enable = json_get(cfg, "enable");

        for (int i = 0; i < (int)enable.size(); ++ i)
        {
            std::string val = json_get<std::string>(enable, i);
            if (val == "all")
            {
                enable_dbg =
                enable_warn =
                enable_err =
                enable_info = true;
            }
            else if (val == "debug")
                enable_dbg = true;
            else if (val == "info")
                enable_info = true;
            else if (val == "warning")
                enable_warn = true;
            else if (val == "error")
                enable_err = true;
            else
                throw_runtime_error("undefined parameter value: ", val);
        }

        __enable_dbg = enable_dbg;
        __enable_warn = enable_warn;
        __enable_err = enable_err;
        __enable_info = enable_info;
    }
}
