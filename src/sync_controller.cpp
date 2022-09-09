#include <cppmisc/traces.h>
#include <cppmisc/argparse.h>
#include "butterfly.h"
#include "overturn_control.h"
#include "serializer.h"

using namespace std;


class ButterflySrv
{
private:
    std::shared_ptr<Connection> m_con;

public:
    ButterflySrv(Json::Value const& cfg)
    {
        auto bflycfg = json_get<Json::Value>(cfg, "sync_butterfly_server");

        int port;
        json_get(bflycfg, "port", port);

        TCPSrv srv(port);
        m_con = srv.wait_for_connection();
        if (!m_con)
            throw_runtime_error("incoming connection error");
    }

    bool send(double ts, double theta, double phi, double dtheta, double dphi)
    {
        char buf[1024];
        int len = ser::pack(buf, sizeof(buf),
            "ts", ts,
            "theta", theta,
            "phi", phi,
            "dtheta", dtheta,
            "dphi", dphi
        );
        if (len <= 0)
        {
            err_msg("ser::pack returned ", len);
            return false;
        }

        int status = m_con->write(buf, len);
        if (status <= 0)
        {
            err_msg("m_con->write returned ", status);
            return false;
        }

        return true;
    }
};

class ButterflyClient
{
private:
    ser::PacketReaderPtr m_pack_reader;
    double  m_theta;
    double  m_ts;
    double  m_phi;
    double  m_dtheta;
    double  m_dphi;

public:
    ButterflyClient(Json::Value const& jscfg)
    {
        auto jsbfly = json_get<Json::Value>(jscfg, "sync_butterfly_client");
        auto ip = json_get<std::string>(jsbfly, "server_ip");
        int port = json_get(jsbfly, "server_port", 0, 65535);

        auto cnct = Connection::connect(ip, port);
        m_pack_reader = ser::make_pack_reader([cnct](char* buf, int bufsz) {
            return cnct->read(buf, bufsz, false);
        });

        m_theta = 0.0;
        m_phi = 0.0;
        m_dtheta = 0.0;
        m_dphi = 0.0;
    }

    bool get(int64_t& ts, double& theta, double& phi, double& dtheta, double& dphi)
    {
        ser::Packet pack;
        int status = m_pack_reader->fetch_next(pack);
        if (status < 0)
        {
            err_msg("m_pack_reader->fetch_next returned ", status);
            return false;
        }
        else if (status > 0)
        {
            info_msg("obtained pack: ", pack.nentries());
            status = pack.get("ts", m_ts, "theta", m_theta, "phi", m_phi, "dtheta", m_dtheta, "dphi", m_dphi);
            if (status <= 0)
            {
                err_msg("pack.get returned ", status);
                return false;
            }
        }

        theta = m_theta;
        phi = m_phi;
        dtheta = m_dtheta;
        dphi = m_dphi;
        return true;
    }
};

int launch(Json::Value const& cfg)
{
    Butterfly bfly;
    bool stop = false;

    auto sig_handler = [&stop,&bfly]() { stop = true; bfly.stop(); };
    SysSignals::instance().set_sigint_handler(stop_handler);
    SysSignals::instance().set_sigterm_handler(stop_handler);
    bfly.init(cfg);

    if (cfg.has("sync_butterfly_server"))
    {
        ButterflySrv bfly_srv(cfg);

        auto f = [&bfly_srv,stop](BflySignals& signals)
        {
            if (stop)
            {
                return false;
            }

            if (!signals.ball_found)
            {
                err_msg("ball was lost");
                return false;
            }

            if (!bfly_srv.send(signals.t, signals.theta, signals.phi, signals.dtheta, signals.dphi))
            {
                err_msg("can't send data to client butterfly");
                return false;
            }

            signals.torque = get_torque(
                signals.theta, signals.phi, signals.dtheta, signals.dphi
            );
            signals.torque = clamp(signals.torque, -0.1, 0.1);
            info_msg("t=", signals.t, ",torque=", signals.torque, ",theta=", signals.theta, ",phi=", signals.phi, 
                 ",dtheta=", signals.dtheta, ",dphi=", signals.dphi, ",x=", signals.x, ",y=", signals.y);
            return true;
        };

        bfly.start(f);
    }
    else if (jscfg.has<Json::Value>("sync_butterfly_client"))
    {
        ButterflyClient bfly_client(jscfg);

        auto f = [&bfly_client,stop](BflySignals& signals)
        {
            if (stop)
            {
                return false;
            }

            if (!signals.ball_found)
            {
                err_msg("ball was lost");
                return false;
            }

            int64_t ts2;
            double theta2,phi2,dtheta2,dphi2;
            if (!bfly_client.get(ts2, theta2, phi2, dtheta2, dphi2))
            {
                err_msg("butterfly server broke connection");
                return false;
            }

            signals.torque = get_torque_sync(
                signals.theta, signals.phi, signals.dtheta, signals.dphi,
                theta2, phi2, dtheta2, dphi2
            );
            signals.torque = clamp(signals.torque, -0.1, 0.1);
            info_msg("t=", signals.t, ",torque=", signals.torque, ",theta=", signals.theta, ",phi=", signals.phi, 
                 ",dtheta=", signals.dtheta, ",dphi=", signals.dphi, ",x=", signals.x, ",y=", signals.y);
            return true;
        };

        bfly.start(f);
    }
    else
    {
        throw_runtime_error("can't find");
    }

    return 0;
}

int main(int argc, char *argv[])
{
    make_arg_list args({
        {{"-c", "--config"}, "config", "path to json config file", "", true}
    });

    int status = 0;

    try
    {
        auto&& m = args.parse(argc, argv);
        Json::Value const& cfg = json_load(m["config"]);
        traces::init(json_get(cfg, "traces"));
        launch(cfg);
    }
    catch (exception const& e)
    {
        err_msg(e.what());
        status = -1;
    }
    catch (...)
    {
        err_msg("Unknown error occured");
        status = -1;
    }

    return status;
}

