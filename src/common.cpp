#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unordered_map>
#include <cppmisc/traces.h>


using namespace std;


int str2int(char const* s)
{
    int val = 0;
    bool sign = *s == '-';

    s += sign;

    while (isdigit(*s))
    {
        val *= 10;
        val += *s - '0';
        ++ s;
    }

    return sign ? -val : val;
}

int int2str(int val, char* s)
{
    if (val == 0)
    {
        *s = '0';
        ++s;
        *s = '\0';
        return 1;
    }

    int len = 0;
    bool sign = val < 0;
    val = sign ? -val : val;

    *s = '-';
    s += sign;
    len += sign;

    int k = 1000000000;

    while (val < k)
    {
        k /= 10;
    }

    while (k != 0)
    {
        *s = (val / k) % 10 + '0';
        ++s;
        ++len;
        k /= 10;
    }

    *s = '\0';
    return len;
}

int64_t __beginning_epoch_usec()
{
    static const int64_t t0 = __epoch_usec();
    return t0;
}

/*
 * files
 */

int open_file(string const& path, string const& mode)
{
    int flags = 0;

    if (mode == "read")
        flags = O_RDONLY;
    else if (mode == "write")
        flags = O_SYNC | O_WRONLY;
    else if (mode == "read-write")
        flags = O_SYNC | O_RDWR;
    else if (mode == "append")
        flags = O_APPEND;
    else
        throw_runtime_error("incorrect value of parameter 'mode': ", mode);

    int f = open(path.c_str(), flags);
    int e = errno;
    if (f < 0)
        throw_runtime_error("can't access ", path, "; error: ", strerror(e));

    dbg_msg("File opened ", path, " ", f);
    return f;
}

void close_file(int fd)
{
    dbg_msg("File closed ", fd);
    close(fd);
}

void write_file(string const& path, string const& data)
{
    int f = open_file(path, "write");
    write_str(f, data.c_str(), data.size());
    close_file(f);
}

void write_file(string const& path, int val)
{
    write_file(path, to_string(val));
}

string read_file(string const& path)
{
    int f = open_file(path, "read");

    string buffer(256, '\0');
    size_t pos = 0;

    while (true)
    {
        size_t n = buffer.size() - pos;
        ssize_t res = read(f, &buffer[pos], n);

        if (res < 0)
        {
            close_file(f);
            throw_runtime_error("can't read from the file ", path);
        }

        if (res == 0)
            break;

        pos += res;
        if (buffer.size() == pos)
            buffer.resize(pos * 2);
    }

    close_file(f);
    buffer.resize(pos);
    return buffer;
}

void write_str(int f, char const* s, int len)
{
    lseek(f, 0, SEEK_SET);
    int res = write(f, s, len);
    throw_if(res != len, runtime_error("can't write to file " + to_string(f) + "id"));
}

int read_int(int f)
{
    char str[64];
    int len;

    lseek(f, 0, SEEK_SET);
    len = read(f, str, sizeof(str));
    throw_if(len <= 0, runtime_error("can't read file " + to_string(f) + "id"));

    return str2int(str);
}

void write_int(int f, int val)
{
    char str[20];
    int len = int2str(val, str);
    write_str(f, str, len);
}

/*
 * devices
 */

const string  slots = "/sys/devices/platform/bone_capemgr/slots";

bool is_device_loaded(string const& device_name)
{
    ifstream f(slots);
    bool found = false;

    if (!f.good())
        throw_runtime_error("can't open ", slots);

    while (!f.eof())
    {
        string line;
        getline(f, line);

        found = line.find(device_name) != string::npos;
        if (found)
            break;
    }

    return found;
}

void load_device(string const& device_name)
{
    write_file(slots, device_name);
}

bool is_file(string const& path)
{
    struct stat sb;
    return stat(path.c_str(), &sb) == 0;
}


/*
 * SIGNALS
 */
static std::unordered_map<int, weak_ptr<signal_handler_t>> __handlers;

static void common_sig_handler(int sig)
{
    auto entry = __handlers.find(sig);

    if (entry == __handlers.end())
    {
        dbg_msg("no signal handler for ", sig);
        return;
    }

    auto f = entry->second.lock();
    if (f)
    {
        dbg_msg("processing signal ", sig);
        (*f)();
    }
    else
    {
        dbg_msg("signal handler for ", sig, " is no more active");
    }
}

void set_sig_handler(int sig, shared_ptr<signal_handler_t> ptr_handler)
{
    __handlers[sig] = ptr_handler;
    signal(sig, common_sig_handler);
}

void set_sigterm_handler(shared_ptr<signal_handler_t> ptr_handler)
{
    set_sig_handler(SIGTERM, ptr_handler);
}

void set_sigint_handler(shared_ptr<signal_handler_t> ptr_handler)
{
    set_sig_handler(SIGINT, ptr_handler);
}
