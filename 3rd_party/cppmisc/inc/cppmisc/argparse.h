#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <map>
#include <stdexcept>
#include <iomanip>
#include <memory>
#include <sstream>
#include "throws.h"


enum class ArgumentsCount
{
    One, // 1
    AtLeastOne, // 1+
    Optional, // 0,1
    Any // 0+
};

struct Argument
{
    std::vector<std::string> keys;
    std::string name;
    std::string help;
    ArgumentsCount count;
    std::string defval;

    Argument(
        std::string const& key, 
        std::string const& name, 
        std::string const& help = std::string(), 
        std::string const& default_value = std::string(), 
        ArgumentsCount count = ArgumentsCount::Optional
    ) :
    keys({ key }), name(name), help(help), count(count), defval(default_value)
    {
    }

    Argument(
        std::initializer_list<char const*> const& keys, 
        std::string const& name, 
        std::string const& help = std::string(),
        std::string const& default_value = std::string(),
        ArgumentsCount count = ArgumentsCount::Optional
    ) :
    name(name), help(help), count(count), defval(default_value)
    {
        this->keys.reserve(keys.size());
        for (auto s : keys)
            this->keys.push_back(s);
    }
};

static std::tuple<std::string, std::string> split(std::string const& s, std::string const& symbols)
{
    auto n = s.find_first_of(symbols);
    if (n == std::string::npos)
        return std::make_tuple(s, "");
    return std::make_tuple(s.substr(0, n), s.substr(n + 1));
}

static std::tuple<std::string, std::string> split_name(std::string const& path)
{
    auto n = path.find_last_of("\\/");
    if (n == path.npos)
        return std::make_tuple("", path);
    return std::make_tuple(path.substr(0, n + 1), path.substr(n + 1));
}

struct ValuesMap
{
private:
    std::map<std::string, std::vector<std::string>> _list;

public:
    inline void append(std::string const& varname, std::string const& value)
    {
        auto p = _list.find(varname);
        if (p == _list.end())
            _list.insert({varname, {value}});
        else
            p->second.push_back(value);
    }

    inline std::string const& get(std::string const& varname, int idx=0) const
    {
        auto p = _list.find(varname);
        if (p == _list.end())
            throw_runtime_error("there is no ", varname, " in the list");
        int sz = p->second.size();
        if (idx >= sz)
            throw_runtime_error("there are only ", sz, " values, but requested ", idx);
        return p->second.at(idx);
    }

    inline int size(std::string const& varname) const
    {
        auto p = _list.find(varname);
        if (p == _list.end())
            return 0;
        return p->second.size();
    }

    inline std::string operator [] (std::string const& varname) const
    {
        return get(varname);
    }

    inline std::map<std::string, std::vector<std::string>>::const_iterator begin() const
    {
        return _list.begin();
    }

    inline std::map<std::string, std::vector<std::string>>::const_iterator end() const
    {
        return _list.end();
    }
};

inline std::ostream& operator << (std::ostream& s, ValuesMap const& m)
{
    for (auto it = m.begin(); it != m.end(); ++ it)
    {
        auto key = it->first;
        auto values = it->second;
        s << key << ": ";
        for (int i = 0; i < values.size(); ++ i)
            s << values[i] << (i == values.size() - 1 ? "" : ", ");
        s << std::endl;
    }
    return s;
}

struct Arguments
{
private:
    std::string program_name;
    std::vector<std::shared_ptr<Argument>> arg_list;
    std::map<std::string, std::shared_ptr<Argument>> args_map;
    int ndefaults;

    void add_arg(Argument const& arg)
    {
        arg_list.push_back(std::make_shared<Argument>(arg));

        for (auto const& key : arg_list.back()->keys)
        {
            if (args_map.find(key) != args_map.end())
                throw_invalid_argument("the multiple definition of the key " + key);

            args_map[key] = arg_list.back();
        }
    }

public:
    Arguments()
    {
        program_name = "program";
    }

    /*
     * typical usage:
     *
     *   make_arg_list args({
     *       {string key, string name, string help_description, string default_value, bool required},
     *       {...}
     *   });
     *   map = args.parse(argc, argv);
     */
    Arguments(std::initializer_list<Argument> const& args, int ndefaults=0)
    {
        program_name = "program";
        this->ndefaults = ndefaults;
        for (auto const& arg: args)
            add_arg(arg);
    }

    std::string help_message() const
    {
        std::stringstream ss;

        ss << "usage: " << std::endl;
        ss << "    " << program_name << " ";
        for (size_t i = 0; i < arg_list.size(); ++i)
        {
            if (arg_list[i]->count == ArgumentsCount::One || 
                arg_list[i]->count == ArgumentsCount::AtLeastOne)
                ss << arg_list[i]->keys[0] << "=" << arg_list[i]->name << " ";
            else
                ss << "[" << arg_list[i]->keys[0] << "] ";
        }
        for (int i = 0; i < ndefaults; ++ i)
        {
            ss << "arg" << i + 1 << " ";
        }

        ss << std::endl << std::endl;

        ss << "arguments:" << std::endl;
        std::vector<std::string> keys_list;
        std::vector<std::string> help_list;

        keys_list.reserve(args_map.size());
        help_list.reserve(args_map.size());

        for (auto const& e : arg_list)
        {
            std::string keys;
            for (size_t i = 0; i < e->keys.size(); ++i)
            {
                if (i == e->keys.size() - 1)
                    keys += e->keys[i];
                else
                    keys += e->keys[i] + ", ";
            }

            keys_list.push_back(keys);
            help_list.push_back(e->help);
        }

        std::string::size_type max_len = 0;
        for (auto const& s : keys_list)
            max_len = std::max(s.size(), max_len);

        for (size_t i = 0; i < keys_list.size(); ++i)
            ss << "    " << std::left << std::setw(max_len + 2) << keys_list[i] << help_list[i] << std::endl;

        return ss.str();
    }

    ValuesMap parse(int argc, char const** argv)
    {
        ValuesMap result;

        if (argc == 0)
            return result;

        std::tie(std::ignore, program_name) = split_name(argv[0]);

        for (auto const& e : args_map)
        {
            if (!e.second->defval.empty())
                result.append(e.second->name, e.second->defval);
        }

        for (int i = 1; i < argc; ++i)
        {
            char const* arg = argv[i];
            auto ans = split(arg, "=");

            if (std::get<1>(ans) == "")
            {
                std::string key = std::get<0>(ans);

                if (args_map.find(key) == args_map.end())
                {
                    // format: value1 value2 ...
                    result.append("rest", std::get<0>(ans));
                }
                else
                {
                    // format: key1 value1 key2 value2 ...
                    if (i == argc - 1)
                        throw_invalid_argument("argument ", key, " is undefined");

                    ++i;
                    std::string name = args_map.at(key)->name;
                    result.append(name, argv[i]);
                }
            }
            // format: key1=value1 key2=value2 ...
            else
            {
                std::string key = std::get<0>(ans);
                if (args_map.find(key) == args_map.end())
                    throw_invalid_argument("don't know the flag: ", std::get<0>(ans));

                std::string name = args_map.at(key)->name;
                result.append(name, std::get<1>(ans));
            }
        }

        for (auto const& e : args_map)
        {
            if (e.second->count == ArgumentsCount::One)
            {
                if (result.size(e.second->name) != 1)
                    throw_invalid_argument("required argument " + e.second->name + " is not specified");
            }
            else if (e.second->count == ArgumentsCount::AtLeastOne)
            {
                if (result.size(e.second->name) == 0)
                    throw_invalid_argument("required argument " + e.second->name + " is not specified");
            }
        }

        return result;
    }
};
