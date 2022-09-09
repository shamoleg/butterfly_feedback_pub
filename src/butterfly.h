#pragma once

#include <memory>
#include <stdexcept>
#include <cppmisc/json.h>
#include <vector>
#include "filters.h"
#include "servo_iface.h"
#include "cam_iface.h"

class FeedbackConfig{
public:
    void fill_from_parse(Json::Value const& jsfbcfg);

    float m_ball;

    std::vector<double> theta;
    std::vector<double> dtheta;
    std::vector<double> phi;
    std::vector<double> dphi;
    std::vector<double> t;

    std::vector<double> sc_t;
    std::vector<double> sc_c;
    int sc_k;

    std::vector<double> k_t;
    std::vector<double> k_c1;
    std::vector<double> k_c2;
    std::vector<double> k_c3;
    int k_k;
};

struct BflySignals
{
    bool ball_found;
    double t;
    double theta;
    double dtheta;
    double phi;
    double dphi;
    double x;
    double vx;
    double y;
    double vy;
    double torque;
};

class Butterfly
{
private:
    std::shared_ptr<ServoIfc> m_servo;
    std::shared_ptr<Camera> m_camera;

    EulerDiff   m_diff_x;
    EulerDiff   m_diff_y;

    double      m_theta, m_dtheta;
    double      m_x, m_y;
    double      m_vx, m_vy;
    double      m_phi, m_dphi;
    bool        m_stop;
    bool        m_ball_found;

    void measure();
    void get_signals(int64_t const& t, BflySignals& signals);

public:
    typedef std::function<bool(BflySignals&, FeedbackConfig&)> callback_t;

    FeedbackConfig fbcfg;

    Butterfly();
    ~Butterfly();

    void init(Json::Value const& jscfg, Json::Value const& jsfbcfg);
    void stop();
    void start(callback_t const& cb);    
};
