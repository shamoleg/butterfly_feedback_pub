#include <iostream>
#include <thread>
#include <unistd.h>
#include <memory.h>
#include <cppmisc/traces.h>
#include <cppmisc/throws.h>
#include <networking/udp.h>


void test_reading_from_any_remote()
{
    info_msg("test_reading_from_any_remote");
    char _test_data[] = "123\000345\nxyz";
    std::string test_data(_test_data, sizeof(_test_data) - 1);

    Udp writer("localhost", 10992);
    Udp reader(10992);

    bool status = writer.write(test_data);
    if (!status)
        throw_runtime_error("can't send: ", writer.status());

    std::string buf;
    status = reader.read(buf, false);
    if (!status)
        throw_runtime_error("can't recv: ", reader.status());

    if (buf.compare(test_data))
        throw_runtime_error("transferred data currupted");

    info_msg("ok");
}

void test_reading_from_specific_remote()
{
    info_msg("test_reading_from_specific_remote");
    char _test_data[] = "123\000345\nxyz";
    std::string test_data(_test_data, sizeof(_test_data) - 1);

    Udp writer("localhost", 10992, 10993);
    Udp reader("localhost", 10993, 10992);

    bool status = writer.write(test_data);
    if (!status)
        throw_runtime_error("can't send: ", writer.status());

    std::string buf;
    status = reader.read(buf, false);
    if (!status)
        throw_runtime_error("can't recv: ", reader.status());

    if (buf.compare(test_data))
        throw_runtime_error("transferred data currupted");

    info_msg("ok");
}

void test_dont_read_from_different()
{
    info_msg("test_dont_read_from_different");
    char _test_data[] = "123\000345\nxyz";
    std::string test_data(_test_data, sizeof(_test_data) - 1);

    const std::string peer1_ip = "127.0.0.1";
    const int peer1_port = 10992;
    const std::string peer2_ip = "127.0.0.1";
    const int peer2_port = 10998;

    Udp peer1(peer2_ip, peer2_port, peer1_port);
    Udp peer2(peer1_ip, peer1_port+1, peer2_port);

    bool status = peer1.write(test_data);
    if (!status)
        throw_runtime_error("can't send: ", peer1.status());

    std::string buf;
    status = peer2.read(buf, false);
    if (!status)
        throw_runtime_error("can't recv: ", peer2.status());

    if (buf.size() != 0)
        throw_runtime_error("received from forbidden ip: ", buf);

    info_msg("ok");
}

int main(int argc, char const* argv[])
{
    try
    {
        test_reading_from_any_remote();
        test_reading_from_specific_remote();
        test_dont_read_from_different();
    }
    catch (std::exception const& e)
    {
        err_msg("failed: ", e.what());
        return -1;
    }

    return 0;
}
