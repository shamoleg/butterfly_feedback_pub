#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cppmisc/formatting.h>
#include <cppmisc/json.h>


Json::Value json_load(std::string const& filepath)
{
    Json::Value json;
    Json::Reader reader;
    std::ifstream f(filepath);
    if (!f.good())
        throw_runtime_error("can't read ", filepath);

    bool ok = reader.parse(f, json);
    if (!ok)
        throw_runtime_error("failed to parse file ", filepath, ": ", reader.getFormattedErrorMessages());

    return json;
}

Json::Value json_parse(char const* str)
{
    Json::Value json;
    Json::Reader reader;
    bool ok = reader.parse(str, json);
    if (!ok)
        throw_runtime_error("failed to parse: ", reader.getFormattedErrorMessages());

    return json;
}
