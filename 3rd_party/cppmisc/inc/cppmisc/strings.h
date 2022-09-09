#include <string>


inline std::string stripstr(std::string const& s)
{
    size_t e1 = s.find_first_not_of(" \t\n\r");
    if (e1 == std::string::npos)
        return "";

    size_t e2 = s.find_last_not_of(" \t\n\r");
    return s.substr(e1, e2 - e1 + 1);
}

inline std::string tolower(std::string const& s)
{
    std::string result = s;
    for (auto& e : result)
        e = std::tolower(e);
    return result;
}

inline int compare_case_insensitive(std::string const& s1, std::string const& s2)
{
    return tolower(s1).compare(tolower(s2));
}

inline bool endswith(std::string const& str, std::string const& ending)
{
    if (str.size() < ending.size())
        return false;
    return str.compare(str.size() - ending.size(), ending.size(), ending) == 0;
}

bool match_mask(std::string const& filename, std::string const& mask);
std::string get_digits_substr(std::string const& s);

inline bool alldigits(char const* s, int len)
{
    for (int i = 0; i < len; ++ i)
    {
        if (!isdigit(s[i]))
            return false;
    }
    return true;
}

inline bool alldigits(std::string const& s)
{
    return alldigits(s.data(), s.size());
}
