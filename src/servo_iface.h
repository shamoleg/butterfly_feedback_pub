#pragma once

#include <memory>
#include <networking/tcp.h>
#include <cppmisc/json.h>
#include <cppmisc/timing.h>
#include "device_manager.h"
#include "servo_protocol.h"


class ServoIfc
{
private:
    std::shared_ptr<Connection> m_connection;
    std::string     m_server_ip;
    int             m_server_port;

    ServoIfc() {}
    ServoIfc(ServoIfc const&) = delete;

public:
    void init(Json::Value const& jscfg)
    {
        auto const& servocfg = json_get<Json::Value>(jscfg, "servo");
        json_get(servocfg, "ip", m_server_ip);
        json_get(servocfg, "port", m_server_port);
    }

    void start()
    {
        m_connection = Connection::connect(m_server_ip, m_server_port);
        if (!m_connection)
            throw_runtime_error("can't connect to ", m_server_ip, ":", m_server_port);

        int64_t t = epoch_usec();
        Servo::CmdPack pack;
        Servo::init_cmd_start(t, pack);
        if (!m_connection->write(reinterpret_cast<char*>(&pack), sizeof(pack)))
            throw_runtime_error("servo connection broken");
    }

    void stop()
    {
        m_connection.reset();
    }

    /*
     * return value:
     *  -1 -- connection closed
     *   0 -- no data received (non-blocking mode)
     *   1 -- new data received
     */
    int get_state(int64_t& t, double& theta, double& dtheta, bool blocking)
    {
        if (!m_connection)
            throw_runtime_error("can't read state: not connected");

        Servo::InfoPack ans;
        int status = m_connection->read(reinterpret_cast<char*>(&ans), sizeof(ans), blocking);
        if (status == 0)
        {
            if (blocking)
            {
                dbg_msg("connection was closed by server");
                return -1;
            }
            return 0;
        }

        if (status < 0)
            throw_runtime_error("can't read from server");

        if (status == sizeof(ans))
        {
            if (!Servo::verify_pack(ans))
                throw_runtime_error("server sent corrupted answer");

            t = ans.t;
            theta = ans.theta;
            dtheta = ans.dtheta;
            return 1;
        }

        throw_runtime_error("unexpected answer from server");
    }

    int set_torque(double const& torque)
    {
        if (!m_connection)
        {
            err_msg("can't set torque: not connected");
            return -1;
        }

        int64_t t = epoch_usec();
        Servo::CmdPack pack;
        Servo::init_cmd_torque(t, torque, pack);
        if (!m_connection->write(reinterpret_cast<char*>(&pack), sizeof(pack)))
        {
            err_msg("can't send packet to server");
            return -1;
        }

        return 0;
    }

    static std::shared_ptr<ServoIfc> capture_instance()
    {
        auto& devices = Devices::get_instance();
        auto instance = devices.capture_instance<ServoIfc>();
        if (instance == nullptr)
        {
            instance.reset(new ServoIfc());
            devices.append(instance);
        }

        return instance;
    }
};
