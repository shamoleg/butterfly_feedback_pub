#include <cppmisc/files.h>
#include <cppmisc/throws.h>
#include <cppmisc/strings.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <tuple>
#include <string>
#include <fstream>


using namespace std;

tuple<string, string> splitname(string const& path)
{
    auto n = path.find_last_of("/\\");
    if (n == string::npos)
        return make_tuple("", path);
    n += 1;
    return make_tuple(path.substr(0, n), path.substr(n));
}

tuple<string, string> splitext(string const& path)
{
    auto n = path.find_last_of("/\\.");
    if (n == string::npos)
        return make_tuple(path, "");

    if (path[n] == '/' || path[n] == '\\')
        return make_tuple(path, "");

    return make_tuple(path.substr(0, n), path.substr(n));
}

string getname(string const& path)
{
    auto n = path.find_last_of("/\\");
    if (n == string::npos)
        return "";
    return path.substr(n + 1);
}

string getext(string const& path)
{
    auto n = path.find_last_of("/\\.");
    if (n == string::npos)
        return "";

    if (path[n] == '/' || path[n] == '\\')
        return "";

    return path.substr(n);
}

string read_all(string const& file)
{
    ifstream f(file);
    if (!f.good())
        throw_runtime_error("can't open ", file);

    f.seekg(0, f.end);
    int length = f.tellg();
    f.seekg(0, f.beg);

    if (length < 0)
        throw_runtime_error("failed requiring file size");

    string data;
    data.resize(length);
    f.read(&data[0], data.size());
    if (!f.good())
        throw_runtime_error("failed to read file ", file);

    return data;
}

void writefile(std::string const& path, std::string const& data)
{
    writefile(path, data.data(), data.size());
}

void writefile(std::string const& path, void const* data, size_t size)
{
    FILE* f = fopen(path.c_str(), "w");
    if (!f)
        throw_runtime_error("can't open ", path, " for writing");
    size_t n = fwrite(data, 1, size, f);
    fclose(f);
    if (n != size)
        throw_runtime_error("can't write to ", path);
}

std::vector<std::string> get_files(std::string const& mask)
{
    vector<string> files;

    string dirpath;
    string namemask;
    tie(dirpath, namemask) = splitname(mask);

    DIR* d = opendir(dirpath.c_str());
    if (!d)
        throw_runtime_error("can't open dir '", dirpath, "'");

    do
    {
        dirent *pentry = readdir(d);

        if (!pentry)
            break;

        if (!match_mask(pentry->d_name, namemask))
            continue;

        auto filepath = dirpath + string(pentry->d_name);

        struct stat s;
        int status = stat(filepath.c_str(), &s);
        if (status)
            continue;

        if (S_ISREG(s.st_mode))
        {
            files.push_back(filepath);
        }
    } 
    while (true);

    closedir(d);

    return files;
}

