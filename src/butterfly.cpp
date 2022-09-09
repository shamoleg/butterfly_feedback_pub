#include "butterfly.h"
#include "filters.h"
#include "string.h"

using namespace std;

void FeedbackConfig::fill_from_parse(Json::Value const& jsfbcfg)
{

    auto const& cfg_parameters = json_get(jsfbcfg, "parameters");

    auto const& cfg_m_ball = json_get(cfg_parameters, "m_ball");
    json_parse(cfg_m_ball, m_ball);


    auto const& cfg_transverse_feedback = json_get(jsfbcfg, "transverse_feedback"); 

    auto const& cfg_traj = json_get(cfg_transverse_feedback, "traj");

    auto const& cfg_theta = json_get(cfg_traj, "theta");
    json_parse(cfg_theta, theta);
    auto const& cfg_dtheta = json_get(cfg_traj, "dtheta");
    json_parse(cfg_dtheta, dtheta);

    auto const& cfg_phi = json_get(cfg_traj, "phi");
    json_parse(cfg_phi, phi);
    auto const& cfg_dphi = json_get(cfg_traj, "dphi");
    json_parse(cfg_dphi, dphi);

    auto const& cfg_t = json_get(cfg_traj, "t");
    json_parse(cfg_t, t);


    // auto const& cfg_servo_constraint = json_get(cfg_transverse_feedback, "servo_constraint");
    
    // auto const& cfg_sc_t = json_get(cfg_servo_constraint, "phi");
    // json_parse(cfg_sc_t, sc_t);

    // auto const& cfg_sc_c = json_get(cfg_servo_constraint, "theta");
    // json_parse(cfg_sc_c, sc_c);

    // auto const& cfg_sc_k = json_get(cfg_servo_constraint, "k");
    // json_parse(cfg_sc_k, sc_k);


    auto const& cfg_k = json_get(cfg_transverse_feedback, "K");
    
    auto const& cfg_k_t = json_get(cfg_k, "t");
    json_parse(cfg_k_t, k_t);

    auto const& cfg_k_c1 = json_get(cfg_k, "k1");
    json_parse(cfg_k_c1, k_c1);

    auto const& cfg_k_c2 = json_get(cfg_k, "k2");
    json_parse(cfg_k_c2, k_c2);

    auto const& cfg_k_c3 = json_get(cfg_k, "k3");
    json_parse(cfg_k_c3, k_c3);

    // auto const& cfg_k_k = json_get(cfg_k, "k");
    // json_parse(cfg_k_k, k_k);

    
    
}

Butterfly::Butterfly()
{
    m_theta = 0;
    m_dtheta = 0;
    m_x = 0;
    m_y = 0;
    m_vx = 0;
    m_vy = 0;
    m_phi = 0;
    m_dphi = 0;
    m_stop = false;
    m_ball_found = false;
}

Butterfly::~Butterfly()
{
}

void Butterfly::init(Json::Value const& cfg, Json::Value const& jsfbcfg)
{
    info_msg("parse feedback..");
    fbcfg.fill_from_parse(jsfbcfg);

    info_msg("initializing hardware..");

    auto const& butcfg = json_get(cfg, "controller");

    m_servo = ServoIfc::capture_instance();
    m_servo->init(cfg);

    m_camera = Camera::capture_instance();
    m_camera->init(cfg);

    info_msg("done");
}

void wait_for_camera_ready()
{

}

void Butterfly::measure()
{
    int64_t t_servo;
    int status = m_servo->get_state(t_servo, m_theta, m_dtheta, true);
    if (status < 0)
        throw_runtime_error("servo disconnected");

    int64_t t_cam;
    status = m_camera->get(t_cam, m_x, m_y);

    switch (status)
    {
    case 1:
    {
        m_vx = m_diff_x.process(t_cam, m_x);
        m_vy = m_diff_y.process(t_cam, m_y);

        double alpha = atan2(m_x, m_y);
        double dalpha = (m_y * m_vx - m_x * m_vy) / (m_x * m_x + m_y * m_y);

        m_phi = m_theta + alpha;
        m_dphi = m_dtheta + dalpha;
        m_ball_found = true;
        break;
    }
    case 0:
    {
        break;
    }
    default:
    {
        if (m_ball_found)
            info_msg("ball was lost");
        m_ball_found = false;
        break;
    }
    }
}

void Butterfly::stop()
{
    m_stop = true;
}

void Butterfly::get_signals(int64_t const& t, BflySignals& signals)
{
    signals.t = t * 1e-6;
    signals.ball_found = m_ball_found;
    signals.theta = m_theta;
    signals.dtheta = m_dtheta;
    signals.phi = m_phi;
    signals.dphi = m_dphi;
    signals.x = m_x;
    signals.vx = m_vx;
    signals.y = m_y;
    signals.vy = m_vy;
    signals.torque = 0;
}

void Butterfly::start(callback_t const& cb)
{
    if (!m_camera || !m_servo)
        throw_runtime_error("Butterfly not initialized yet");

    m_camera->start();
    m_servo->start();

    int status;
    int64_t t, t0;
    t0 = epoch_usec();

    while (!m_stop)
    {
        t = epoch_usec();
        measure();

        BflySignals signals;
        get_signals(t - t0, signals);

        // attention dirty
        status = cb(signals, fbcfg);
            
        if (!status)
            m_stop = true;

        m_servo->set_torque(signals.torque);
    }

    m_servo->stop();
    m_camera->stop();

    info_msg("stopped");
}
