#pragma once

#include <string>
#include <memory>
#include <networking/tcp.h>
#include <cppmisc/json.h>
#include "serializer.h"


class Camera
{
private:
    ser::PacketReaderPtr con_reader;
    std::string     host;
    int             port;

    Camera();
    Camera(Camera const&) = delete;

public:
    ~Camera();

    void init(Json::Value const& jscfg);

    // 1 -- new measurement obtained
    // 0 -- idle
    // -1 -- ball wasn't detected
    int get(int64_t& ts_usec, double& x, double& y);

    void start();
    void stop();

    static std::shared_ptr<Camera> capture_instance();
};

