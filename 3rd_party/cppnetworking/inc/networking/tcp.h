#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <vector>


class TCPSrv;
class Connection;

using ConnectionPtr = std::shared_ptr<Connection>;
using TCPSrvPtr = std::shared_ptr<TCPSrv>;


/*
 * Connection
 */
 
class Connection
{
private:
    friend class TCPSrv;
    int remote_sock;

    Connection(int remotre_sock);
    Connection(Connection const&);

public:
    Connection(Connection&&);

    ~Connection();

    bool write(char const* s, int len);
    bool write(std::string const& s);
    bool write(std::vector<uint8_t> const& buf);


    int wait_for_data(int64_t usec);
    
    /*
     * returns 
     * -1 failed
     *  0 pending
     *  N number of bytes received
     */
    int read(char* s, int len, bool blocking);
    int read(std::string& buf, bool blocking);
    int read(std::vector<uint8_t>& buf, bool blocking);

    int available();
    std::tuple<int, std::string> read(bool blocking);
    bool alive();

    static ConnectionPtr connect(std::string const& ip, int port);
};


/*
 * tcp server
 */

class TCPSrv
{
private:
    int     port;
    int     srv_sock;

private:
    void init_server();
    void stop_server();

public:
    TCPSrv(int port);
    ~TCPSrv();

    ConnectionPtr wait_for_connection();
    void run();
    void stop();
};
