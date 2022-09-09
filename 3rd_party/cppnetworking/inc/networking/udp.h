#pragma once

#include <string>
#include <arpa/inet.h>
#include <memory>


class Udp;
typedef std::shared_ptr<Udp> UdpPtr;

class Udp
{
private:
    int _sock;
    std::string _status;

public:
    Udp() = delete;
    Udp(Udp const&) = delete;
    Udp(Udp&& udp);

    // 1. UDP for reading from a specific remote; incomming messages from others will be ignored
    // 2. UDP for exchangind data with a specific peer
    Udp(std::string const& remote_ip, int remote_port, int local_port);

    // 3. UDP for writing to a specific remote; reading forbidden
    Udp(std::string const& remote_ip, int remote_port);

    // 4. UDP for reading from any remote; writing forbidden
    Udp(int local_port);

    ~Udp();

    // returns:
    //  true if succeed
    //  false if failed
    bool write(char const* data, int sz);

    // returns:
    //  true if succeed
    //  false if failed
    bool write(std::string const& data);

    // returns:
    //  number of butes received if succeed
    //  -1 if failed
    //  0 if nothing to read (nonblocking case)
    int read(char* buf, int sz, bool blocking=true);

    // returns:
    //  true if succeed
    //  false if failed
    bool read(std::string& buf, bool blocking=true);

    // wait for data not more than 'wait_interval'
    // if 'wait_interval' < 0 wait infinitely long (usual blocking)
    // if 'wait_interval' == 0 don't wait (usual non blocking)
    //  returns:
    //   0 timed out
    //  -1 failed
    //   N bytes read
    int read(char* buf, int sz, int64_t wait_interval);

    // returns:
    //  description of arose error
    std::string const& status() const;
};
