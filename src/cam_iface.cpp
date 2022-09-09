#include <string.h>
#include "cam_iface.h"
#include <cppmisc/traces.h>
#include <cppmisc/timing.h>
#include "device_manager.h"

using namespace std;


Camera::Camera()
{
    host = "";
    port = 0;
}

Camera::~Camera()
{
}

void Camera::init(Json::Value const& jscfg)
{
    auto const& jscam = json_get(jscfg, "camera");
    json_get<std::string>(jscam, "ip", host);
    json_get(jscam, "port", port);
}

void Camera::start()
{
    if (host.empty())
        throw_runtime_error("camera is not initialized yet; run init(...)");

    auto cnct = Connection::connect(host, port);
    con_reader = ser::make_pack_reader([cnct](char* p, int n) {
        if (!cnct)
            return -1;
        return cnct->read(p, n, false);
    });
    char buf[1024];
    int len = ser::pack(buf, sizeof(buf), 
        "ts", epoch_usec(), 
        "cmd", "start"
    );
    cnct->write(buf, len);
}

void Camera::stop()
{
    con_reader = nullptr;
}

int Camera::get(int64_t& ts_usec, double& x, double& y)
{
    if (!con_reader)
        throw_runtime_error("not connected to cumera; call run();");

    ser::Packet pack;
    int status = con_reader->fetch_next(pack);
    if (status < 0)
        throw_runtime_error("connection closed");

    if (status == 0)
        return 0;

    bool good;
    status = pack.get("good", good);
    if (status <= 0)
        throw_runtime_error("camera data corrupted");

    if (!good)
        return -1;

    status = pack.get("x", x, "y", y, "ts", ts_usec);
    if (status <= 0)
        throw_runtime_error("camera data corrupted");

    return 1;
}

shared_ptr<Camera> Camera::capture_instance()
{
    auto& devices = Devices::get_instance();
    auto instance = devices.capture_instance<Camera>();
    if (instance == nullptr)
    {
        instance.reset(new Camera());
        devices.append(instance);
    }

    return instance;
}
