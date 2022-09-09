#include <cppmisc/traces.h>
#include <cppmisc/argparse.h>
#include "../src/servo_iface.h"


void launch(Json::Value const& jscfg)
{
    auto servo = ServoIfc::capture_instance();
    servo->init(jscfg);
    servo->start();

    int64_t t;
    double theta, dtheta;
    int status = servo->get_state(t, theta, dtheta, true);
    if (status < 0)
        throw_runtime_error("received corrupted packed");

    dbg_msg("t=", t, "; theta=", theta, "; dtheta=", dtheta, ";");
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

