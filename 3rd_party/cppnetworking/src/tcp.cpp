#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string.h>
#include <iostream>
#include <stdexcept>

#include <networking/tcp.h>
#include <cppmisc/traces.h>
#include <cppmisc/throws.h>


using namespace std;


/*
 * utils
 */

inline int available_bytes(int sock)
{
    int bytes_available;
    int status = ioctl(sock, FIONREAD, &bytes_available);
    if (status < 0)
        return status;
    return bytes_available;
}

inline bool is_socket_alive(int sock)
{
    int error_code;
    socklen_t error_code_size = sizeof(error_code);
    int status = getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
    if (status < 0)
    {
        err_msg("getsockopt SO_ERROR returned ", status, "; error code ", errno);
        return false;
    }

    return error_code == 0;
}

inline timeval to_timeval(int64_t usec)
{
    timeval tv;
    tv.tv_sec = int(usec / int64_t(1e+6));
    tv.tv_usec = int(usec % int64_t(1e+6));
    return tv;
}
/*
* Connection
*/

Connection::Connection(int remote_sock) : 
	remote_sock(remote_sock)
{
}

Connection::Connection(Connection&& connection)
{
    remote_sock = connection.remote_sock;
    connection.remote_sock = -1;
}

Connection::~Connection()
{
    if (remote_sock != -1)
    {
        shutdown(remote_sock, SHUT_RDWR);
        close(remote_sock);
    }

    dbg_msg("Connection closed");
}

bool Connection::write(char const* s, int len)
{
    int res = send(remote_sock, s, len, MSG_NOSIGNAL);
    return res == len;
}

bool Connection::write(string const& s)
{
    return write(s.data(), s.size());
}

bool Connection::write(std::vector<uint8_t> const& buf)
{
    return write(reinterpret_cast<char const*>(buf.data()), buf.size());
}

int Connection::wait_for_data(int64_t usec){
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(remote_sock, &rfds);

    timeval&& tv = to_timeval(usec);

    return select(remote_sock + 1, &rfds, nullptr, nullptr, &tv);
}

int Connection::read(char* s, int len, bool blocking)
{
    int flags = blocking ? 0 : MSG_DONTWAIT;
    int status = recv(remote_sock, s, len, flags);
    if (status > 0)
    {
        return status;
    }
    else if (status < 0)
    {
        if (!blocking && (errno == EAGAIN || errno == EWOULDBLOCK))
            return 0;
        dbg_msg("an error occurred");
        return -1;
    }
    else // status == 0
    {
        dbg_msg("connection closed");
        return -1;
    }
}

tuple<int, string> Connection::read(bool blocking)
{
    char buf[1024];
    int len = read(buf, sizeof(buf), blocking);
    if (len < 0)
        return make_tuple(-1, "");

    if (len == 0)
        return make_tuple(0, "");

    return make_tuple(1, string(buf, len));
}

int Connection::read(std::vector<uint8_t>& buf, bool blocking)
{
    buf.resize(1024);
    int status = read(reinterpret_cast<char*>(&buf[0]), buf.size(), blocking);
    if (status < 0)
    {
        buf.resize(0);
        return status;
    }
    else if (status > 0)
    {
        buf.resize(status);
        return status;
    }
    else
    {
        return 0;
    }
}

int Connection::read(std::string& buf, bool blocking)
{
    buf.resize(1024);
    int status = read(&buf[0], buf.size(), blocking);
    if (status < 0)
    {
        buf.resize(0);
        return status;
    }
    else if (status > 0)
    {
        buf.resize(status);
        return status;
    }
    else
    {
        return 0;
    }
}

std::shared_ptr<Connection> Connection::connect(std::string const& ip, int port)
{
    dbg_msg("connecting to the server ", ip, ":", port, "..");
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        throw_runtime_error("can't open socket");

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    int syn_retries = 3;
    setsockopt(sock, IPPROTO_TCP, TCP_SYNCNT, &syn_retries, sizeof(syn_retries));

    int res = ::connect(sock, (sockaddr const*)&addr, sizeof(addr));
    if (res < 0)
    {
        close(sock);
        throw_runtime_error("can't connect to host '" + ip + ":" + to_string(port) + "'");
    }

    dbg_msg("connected");
    return std::shared_ptr<Connection>(new Connection(sock));
}


/*
 * TCPSrv
 */

TCPSrv::TCPSrv(int port) : 
    port(port), srv_sock(-1)
{
    init_server();
}

TCPSrv::~TCPSrv()
{
    if (srv_sock >= 0)
    {
        shutdown(srv_sock, SHUT_RDWR);
        close(srv_sock);
        srv_sock = -1;
    }
}

shared_ptr<Connection> TCPSrv::wait_for_connection()
{
    int res = listen(srv_sock, SOCK_STREAM);
    if(res)
        throw_runtime_error("listen socket error: ", strerror(errno), " for socket ", srv_sock);

    sockaddr_in remote_addr;
    socklen_t addr_len = sizeof(remote_addr);

    while (true)
    {
        int remote_sock = accept(srv_sock, (sockaddr*)&remote_addr, &addr_len);

        if (remote_sock < 0 && errno == EAGAIN)
        {
            dbg_msg("something is wrong with socket accepting ", strerror(errno), ", retrtying..");
            continue;
        }

        if (remote_sock < 0 && errno == EINVAL)
            return nullptr;

        if (remote_sock < 0)
            throw_runtime_error("can't accept Connection");

        shared_ptr<Connection> ptr;
        ptr.reset(new Connection(remote_sock));
        return ptr;
    }
}

void TCPSrv::stop()
{
    if (srv_sock >= 0)
        shutdown(srv_sock, SHUT_RDWR);
}

void TCPSrv::init_server()
{
    srv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (srv_sock < 0)
        throw_runtime_error("can't open socket");

    sockaddr_in localhost;
    memset(&localhost, 0, sizeof(localhost));
    localhost.sin_port = htons(port);
    localhost.sin_family = AF_INET;
    localhost.sin_addr.s_addr = INADDR_ANY;

    int res = bind(srv_sock, (sockaddr const*)&localhost, sizeof(localhost));
    if (res)
    {
        close(srv_sock);
        srv_sock = -1;
        throw_runtime_error("can't bind socket to localhost:", port, "; reason: ", strerror(errno));
    }
}
