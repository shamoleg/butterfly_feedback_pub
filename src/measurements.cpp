#include <functional>
#include <cppmisc/traces.h>
#include "servo_iface.h"
#include <cppmisc/argparse.h>
#include <cppmisc/threads.h>
#include "matrix.h"
#include "filters.h"


using Feedback = std::function<double(double, double, double)>;


std::tuple<bool,double> sin_feedback(int64_t ts, double theta, double dtheta)
{
    double w = 10.;
    double t = usec_to_sec(ts);
    double torque = -0.03 * sin(w*t) - 0.003;
    info_msg(t, " ", torque, " ", theta, " ", dtheta);
    return std::make_tuple(t > 10, torque);
}


std::tuple<bool,double> set_zero(int64_t t, double theta, double dtheta)
{
    double k1 = 3.;
    double k2 = 0.3;
    double torque = -k1 * clamp(theta, -3., 3.) - k2 * dtheta;
    torque = clamp(torque, -0.05, 0.05);
    info_msg(t, " ", torque, " ", theta, " ", dtheta);
    bool stop = std::fabs(theta) < 1e-2 && std::fabs(dtheta) < 1e-2;
    return std::make_tuple(stop, torque);
}

template <class F>
int launch(Json::Value const& jscfg, F& f)
{
    int status = 0;
    auto servo = ServoIfc::capture_instance();
    servo->init(jscfg);
    servo->start();
    bool stop;

    set_thread_rt_priotiy(-1, 90);

    while (true)
    {
        int64_t t;
        double theta, dtheta;

        status = servo->get_state(t, theta, dtheta, true);
        if (status < 0)
        {
            err_msg("received corrupted data");
            break;
        }

        double torque;
        std::tie(stop,torque) = f(t, theta, dtheta);

        if (stop)
            break;

        if (!in_diap(torque, -0.1, 0.1))
        {
            err_msg("torque is too big");
            break;
        }

        servo->set_torque(torque);
    }

    servo->set_torque(0.0);
    servo->stop();
    return status;
}


int main(int argc, char const* argv[])
{
    Arguments args({
        Argument("-c", "config", "path to json config file", "", ArgumentsCount::One)
    });

    int status = 0;

    try
    {
        auto&& m = args.parse(argc, argv);
        Json::Value const& cfg = json_load(m["config"]);
        traces::init(json_get(cfg, "traces"));
        // status = launch(cfg, set_zero);
        status = launch(cfg, sin_feedback);
        // Feedback6 feedback(10.);
        // status = launch(cfg, feedback);
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
