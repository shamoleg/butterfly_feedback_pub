#pragma once
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>


inline bool file_exists(char const* filepath)
{
    struct stat64 sb;
    int ans = stat64(filepath, &sb);
    return ans == 0 && S_ISREG(sb.st_mode);
}

inline bool file_exists(std::string const& filepath)
{
    return file_exists(filepath.c_str());
}

inline int64_t file_size(char const* filepath)
{
    struct stat64 sb;
    int ans = stat64(filepath, &sb);
    if (ans < 0)
        return -1;
    return (int64_t)sb.st_size;
}

inline bool dir_exists(char const* dirpath)
{
    struct stat64 sb;
    int ans = stat64(dirpath, &sb);
    return ans == 0 && S_ISDIR(sb.st_mode);
}

inline bool dir_exists(std::string const& dirpath)
{
    return dir_exists(dirpath.c_str());
}

void writefile(std::string const& path, void const* data, size_t size);
void writefile(std::string const& path, std::string const& data);

std::string read_all(std::string const& file);
std::tuple<std::string, std::string> splitname(std::string const& path);
std::tuple<std::string, std::string> splitext(std::string const& path);
std::string getname(std::string const& path);
std::string getext(std::string const& path);
std::vector<std::string> get_files(std::string const& mask);
