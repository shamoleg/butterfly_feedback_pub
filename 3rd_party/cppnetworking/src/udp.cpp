#include <stdexcept>

#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <memory.h>
#include <cppmisc/traces.h>
#include <cppmisc/throws.h>
#include <networking/udp.h>


inline sockaddr_in make_addr(in_addr_t addr, int port)
{
    sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = addr;
    saddr.sin_port = htons(port);
    return saddr;
}

inline sockaddr_in make_addr(std::string const& addr, int port)
{
    if (addr == "localhost")
    {
        return make_addr(INADDR_ANY, port);
    }

    auto a = inet_addr(addr.c_str());
    if (!a)
    {
        auto s = format("invalid address: ", addr);
        throw_runtime_error(s);
    }

    return make_addr(a, port);
}

inline timeval to_timeval(int64_t usec)
{
    timeval tv;
    tv.tv_sec = int(usec / int64_t(1e+6));
    tv.tv_usec = int(usec % int64_t(1e+6));
    return tv;
}


// 1. UDP for reading from a specific remote; incomming messages from others will be ignored
// 2. UDP for exchangind data with a specific peer
Udp::Udp(std::string const& remote_ip, int remote_port, int local_port)
{
    _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sock < 0)
        throw_runtime_error("can't create udp socket ", strerror(errno));

    sockaddr_in local_addr = make_addr(INADDR_ANY, local_port);
    int res = bind(_sock, (sockaddr const*)&local_addr, sizeof(local_addr));
    if (res < 0)
    {
        int err = errno;
        close(_sock);
        _sock = -1;
        throw_runtime_error("can't open port ", local_port, ": ", strerror(err));
    }

    sockaddr_in remote_addr = make_addr(remote_ip, remote_port);    
    res = connect(_sock, (sockaddr const*)&remote_addr, sizeof(remote_addr));
    if (res != 0)
    {
        volatile int err = errno;
        close(_sock);
        _sock = -1;
        throw_runtime_error("connect ", remote_ip, ":", remote_port, " failed: ", strerror(err));
    }
}

// 3. UDP for writing to a specific remote; reading forbidden
Udp::Udp(std::string const& remote_ip, int remote_port)
{
    _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sock < 0)
        throw_runtime_error("can't create udp socket: ", strerror(errno));

    sockaddr_in remote_addr = make_addr(remote_ip, remote_port);    
    int res = connect(_sock, (sockaddr const*)&remote_addr, sizeof(remote_addr));
    if (res != 0)
    {
        volatile int err = errno;
        close(_sock);
        _sock = -1;
        throw_runtime_error("connect ", remote_ip, ":", remote_port, " failed: ", strerror(err));
    }
}

// 4. UDP for reading from any remote; writing forbidden
Udp::Udp(int local_port)
{
    _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sock < 0)
        throw_runtime_error("can't create udp socket ", strerror(errno));

    sockaddr_in local_addr = make_addr(INADDR_ANY, local_port);
    int res = bind(_sock, (sockaddr const*)&local_addr, sizeof(local_addr));
    if (res < 0)
    {
        int err = errno;
        close(_sock);
        _sock = -1;
        throw_runtime_error("can't open port ", local_port, ": ", strerror(err));
    }
}

Udp::Udp(Udp&& udp)
{
    this->_sock = udp._sock;
    this->_status = udp._status;
    udp._sock = -1;
}

Udp::~Udp()
{
    if (_sock > 0)
        close(_sock);
}

bool Udp::write(char const* data, int sz)
{
    int status = send(_sock, data, sz, MSG_NOSIGNAL);
    if (status < 0)
        _status = strerror(errno);
    return status == sz;
}

bool Udp::write(std::string const& data)
{
    return write(&data[0], data.size());
}

int Udp::read(char* buf, int sz, bool blocking)
{
    int flags = blocking ? 0 : MSG_DONTWAIT;
    int status = recv(_sock, buf, sz, flags);
    if (status < 0)
    {
        if (!blocking && errno == EWOULDBLOCK)
            return 0;

        _status = strerror(errno);
        return -1;
    }
    else if (status == 0)
    {
        _status = strerror(errno);
        return -1;
    }
    else
    {
        return status;
    }
}

bool Udp::read(std::string& buf, bool blocking)
{
    buf.resize(4096);
    int status = read(&buf[0], buf.size(), blocking);
    if (status < 0)
    {
        _status = strerror(errno);
        buf.resize(0);
        return false;
    }

    buf.resize(status);
    return true;
}

int Udp::read(char* buf, int sz, int64_t usec)
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(_sock, &rfds);

    if (usec < 0)
        return read(buf, sz, true);
    if (usec == 0)
        return read(buf, sz, false);

    timeval&& tv = to_timeval(usec);

    int retval = select(_sock + 1, &rfds, nullptr, nullptr, &tv);
    if (retval < 0)
    {
        _status = strerror(errno);
        return -1;
    }
    else if (retval == 0)
    {
        return 0;
    }
    else
    {
        return read(buf, sz, false);
    }
}

std::string const& Udp::status() const
{
    return _status;
}
