#include <cppmisc/traces.h>
#include <cppmisc/argparse.h>
#include <cppmisc/signals.h>
#include "butterfly.h"
#include "overturn_controller.h"
#include "vector"



using namespace std;

template<class T>
void print_vector(std::vector<T> v){
    for(auto b : v){
        std::cout << b << " ";
    }
    std::cout << std::endl;
}

template<class T>
std::vector<T> get_vector_from_b(std::vector<T> v, std::vector<bool> b){
    std::vector<T> result;
    for(int i = 0; i < v.size(); ++i){
        if(b.at(i) == 1)
            result.emplace_back(v.at(i));
    }
    return result;
}

double servo_constraint(double phi, double p=0){
    return 10000;
}

static vector<double> get_transverse(BflySignals const& signals, FeedbackConfig const& fbcfg)
{
    double theta = signals.theta;
    double phi = signals.phi;
    double dtheta = signals.dtheta;
    double dphi = signals.dphi;

    std::vector<bool> b;
    if (dphi >= 0)
        for(auto const& i : fbcfg.dphi){
            b.emplace_back(i > 0);
        }
    if (dphi < 0)
        for(auto const& i : fbcfg.dphi){
            b.emplace_back(i < 0);
        }

    std::vector<double> phi_masked_b = get_vector_from_b(fbcfg.phi, b);
    
    std::vector<double> d;
    for(auto const& i : phi_masked_b){
        d.emplace_back(abs(i - phi));
    }

    int i_agrmin = 0;
    double min_val = d.at(0);
    for(int i; i < d.size(); ++i){
        if(d.at(i) < min_val){
            i_agrmin = i;
            min_val = d.at(i);
        }
    }

    std::vector<double> t_masked_b = get_vector_from_b(fbcfg.t, b);
    double tau = t_masked_b.at(i_agrmin);
    
    std::vector<double> dphi_masked_b = get_vector_from_b(fbcfg.dphi, b);

    double dphi_star = dphi_masked_b.at(i_agrmin);
    double I = pow(dphi, 2) - pow(dphi_star, 2);
    double y  = theta - servo_constraint(phi); 
    double dy = dtheta - servo_constraint(phi, 1) * dphi;

    return {tau, y, dy, I};

    // TODO: remove code repetition
}

// static vector<double> get_transverse2(BflySignals const& signals, FeedbackConfig const& fbcfg){
//     double theta = signals.theta;
//     double phi = signals.phi;
//     double dtheta = signals.dtheta;
//     double dphi = signals.dphi;


//     return {tau, y, dy, I};

// }

static double get_torque_sham(BflySignals const& signals, FeedbackConfig const& fbcfg)
{   

        double theta = signals.theta;
        double phi = signals.phi;
        double dtheta = signals.dtheta;
        double dphi = signals.dphi;

        auto n = int(floor(phi / _PI));
        phi = phi - _PI * n;
        theta = theta - _PI * n;

        static const spline s_phi(3, fbcfg.phi, fbcfg.t, "none");
        static const spline s_dphi(3, fbcfg.t, fbcfg.dphi,  "none");
        static const spline s_theta(3, fbcfg.t, fbcfg.theta,  "none");
        static const spline s_dtheta(3, fbcfg.t, fbcfg.dtheta,   "none");
        static const spline s_dtheta_by_phi(3, fbcfg.phi, fbcfg.dtheta,   "none");
        static const spline s_vc1(3, fbcfg.phi, fbcfg.theta,   "none");
        
        auto dtheta_s = s_theta(phi, 1) * s_dphi(phi);
        auto dphi_s = s_dphi(phi);
        auto vc1 = s_vc1(phi, 1);
        auto vc2 = s_vc1(phi, 2);

        auto tau = s_phi(phi);
        auto dphi_star = s_dphi(tau);

        double I = pow(dphi, 2) - pow(dphi_star, 2);
        double y  = theta - s_theta(tau);
        double dy = dtheta - s_dtheta_by_phi(phi, 1) * dphi;

        Vec2 dq_s(s_dtheta(phi), s_dphi(phi));
        auto M = sub_M(theta, phi);
        auto invM = inv(M);
        auto tmp = s_dtheta_by_phi(phi, 1) * s_dphi(phi);
        auto C = sub_C(s_theta(phi), phi, tmp, s_dphi(phi));
        auto G = sub_G(theta, phi);
        auto B = Mat2x1(1/fbcfg.m_ball, 0);

        static const spline s_Theta(fbcfg.sc_k, fbcfg.sc_t, fbcfg.sc_c, "none");
        double Theta1 = s_Theta(phi, 1);
        double Theta2 = s_Theta(phi, 2);
        auto mat_with_theta1 = Mat1x2(1, -Theta1);

        auto a = (mat_with_theta1 * invM * (C * dq_s - G)).at(0.0) -Theta2 * pow(dphi, 2);
        auto b = (mat_with_theta1 * invM * B).at(0.0);

        std::vector<double> tau_y_dy_I = {tau, y, dy, I};

        static const spline s_K1(fbcfg.k_k, fbcfg.k_t, fbcfg.k_c1, "none");
        static const spline s_K2(fbcfg.k_k, fbcfg.k_t, fbcfg.k_c2, "none");
        static const spline s_K3(fbcfg.k_k, fbcfg.k_t, fbcfg.k_c3, "none");
        double var1 = s_K1(tau_y_dy_I.at(0)) * tau_y_dy_I.at(1);
        double var2 = s_K2(tau_y_dy_I.at(0)) * tau_y_dy_I.at(2);
        double var3 = s_K3(tau_y_dy_I.at(0)) * tau_y_dy_I.at(3);
        double v =  var1 + var2 + var3;

        // Vec2 dq_s(dtheta_s, dphi_s);
        Mat2x2 invL(1, -vc1, 0, 1);
        
        auto K = invL * invM;


        // auto u = 1/b * v - a/b;
        // u *= 30;

        auto u = (v + (K * (G + C * dq_s)).at(0, 0) + vc2 * pow(dphi_s, 2)) / K.at(0, 0);
        info_msg("i = ", u);
        return u;

}

static double get_torque_sham2(BflySignals const& signals, FeedbackConfig const& fbcfg)
{   

        double theta = signals.theta;
        double phi = signals.phi;
        double dtheta = signals.dtheta;
        double dphi = signals.dphi;

        // auto n = int(floor(phi / _PI));
        // phi = phi - _PI * n;
        // theta = theta - _PI * n;

        
        static const spline s_phi(3, fbcfg.phi, fbcfg.t, "none");
        static const spline s_dphi(3, fbcfg.phi, fbcfg.dphi,  "none");
        static const spline s_theta(3, fbcfg.phi, fbcfg.theta,  "none");
        static const spline s_dtheta(3, fbcfg.phi, fbcfg.dtheta,   "none");
        //auto tau=s_phi(phi);
        auto dtheta_s = s_theta(phi, 1) * s_dphi(phi);
        auto dphi_s = s_dphi(phi);
        auto vc1 = s_theta(phi, 1);
        auto vc2 = s_theta(phi, 2);

        auto dphi_star = s_dphi(phi);

        double I = pow(dphi, 2) - pow(dphi_star, 2);
        double y  = theta - s_theta(phi);
        double dy = dtheta - s_dtheta(phi, 1) * dphi;

        auto M = sub_M(theta, phi);
        auto invM = inv(M);
        auto tmp = vc1 * s_dphi(phi);
        auto C = sub_C(s_theta(phi), phi, tmp, s_dphi(phi));
        auto G = sub_G(theta, phi);
        auto B = Mat2x1(1/fbcfg.m_ball, 0);

        info_msg("1");
        static const spline s_K1(fbcfg.k_k, fbcfg.phi, fbcfg.k_c1, "none");
        static const spline s_K2(fbcfg.k_k, fbcfg.phi, fbcfg.k_c2, "none");
        static const spline s_K3(fbcfg.k_k, fbcfg.phi, fbcfg.k_c3, "none");
        info_msg("2");


        double v =  s_K1(phi) * y + 
                    s_K2(phi) * dy + 
                    s_K3(phi) * I;

        Vec2 dq_s(dtheta_s, dphi_s);
        Mat2x2 invL(1, -vc1, 0, 1);
        auto K = invL * invM;

        auto u = (v + (K * (G + C * dq_s)).at(0, 0) + vc2 * pow(dphi_s, 2)) / K.at(0, 0);
        info_msg("phi = ", u);
        return u;
}

static double get_torque_sham3(BflySignals const& signals, FeedbackConfig const& fbcfg)
{   

        double theta = signals.theta;
        double phi = signals.phi;
        double dtheta = signals.dtheta;
        double dphi = signals.dphi;

        auto n = int(floor(phi / _PI));
        phi = phi - _PI * n;
        theta = theta - _PI * n;

        static const spline s_dphi(3, fbcfg.phi, fbcfg.dphi,  "none");
        static const spline s_theta(3, fbcfg.phi, fbcfg.theta,  "none");
        static const spline s_dtheta(3, fbcfg.phi, fbcfg.dtheta,   "none");

        auto dphi_star = s_dphi(phi);
        double I = pow(dphi, 2) - pow(dphi_star, 2);
        double y  = theta - s_theta(phi);
        double dy = dtheta - s_dtheta(phi, 1) * dphi;

        auto vc1 = s_theta(phi, 1);
        auto vc2 = s_theta(phi, 2);
        auto tmp = vc1 * s_dphi(phi);

        auto C = sub_C(s_theta(phi), phi, tmp, s_dphi(phi));
        auto G = sub_G(theta, phi);
        auto B = Mat2x1(1/fbcfg.m_ball, 0);
        auto M = sub_M(theta, phi);
        auto invM = inv(M);

        static const spline s_K1(3, fbcfg.phi, fbcfg.k_c1, "none");
        static const spline s_K2(3, fbcfg.phi, fbcfg.k_c2, "none");
        static const spline s_K3(3, fbcfg.phi, fbcfg.k_c3, "none");

        double v =  s_K1(phi) * y + 
                    s_K2(phi) * dy + 
                    s_K3(phi) * I;

        
        Mat2x2 invL(1, -vc1, 0, 1);

        auto K = invL * invM;
        auto dtheta_by_s = s_theta(phi, 1) * s_dphi(phi);
        auto dphi_by_s = s_dphi(phi);
        Vec2 dq_s(dtheta_by_s, dphi_by_s);

        auto u = (v + (K * (G + C * dq_s)).at(0, 0) + vc2 * pow(dphi_by_s, 2)) / K.at(0, 0);
        // auto u = 1/b * v - a/b;
        info_msg("i = ", u, " s_dphi = ", s_dphi(0.2),  " I = ", I);

        info_msg("k1 = ", s_K1(0.4), " k2 = ", s_K2(0.4), " k3 = ", s_K3(0.4));
        return 0;
}

static double get_torque(BflySignals const& signals, FeedbackConfig const& fbcfg)
{

    double theta = signals.theta;
    double phi = signals.phi;
    double dtheta = signals.dtheta;
    double dphi = signals.dphi;

    // info_msg("phi_b = ", phi, " theta_b = ", theta);
    auto n = int(floor(phi / _PI));
    phi -= _PI * n;
    theta -= _PI * n;
    // info_msg("phi = ", phi, " theta = ", theta);
    
    static const spline spline_dphi_sham(3, fbcfg.phi, fbcfg.dphi,  "none");
    static const spline spline_vc_sham(3, fbcfg.phi, fbcfg.theta,  "none");

    auto dphi_s = spline_dphi_sham(phi);
    auto theta_s = spline_vc_sham(phi);
    auto vc1 = spline_vc_sham(phi, 1);
    auto vc2 = spline_vc_sham(phi, 2);
    

    static const spline spline_ky_sham(3, fbcfg.phi, fbcfg.k_c1, "none");
    static const spline spline_kdy_sham(3, fbcfg.phi, fbcfg.k_c2, "none");
    static const spline spline_kz_sham(3, fbcfg.phi, fbcfg.k_c3, "none");
    auto ky = spline_ky_sham(phi);
    auto kdy = spline_kdy_sham(phi);
    auto kz = spline_kz_sham(phi);

    auto dtheta_s = vc1 * dphi_s;

    auto z = dphi - dphi_s;
    auto y = theta - theta_s;
    auto dy = dtheta - vc1 * dphi;

    auto v = z * kz + y * ky + dy * kdy;

    Vec2 dq_s(dtheta_s, dphi_s);
    Mat2x2 invL(1, -vc1, 0, 1);
    auto M = sub_M(theta, phi);
    auto C = sub_C(theta_s, phi, dtheta_s, dphi_s);
    auto G = sub_G(theta, phi);

    auto invM = inv(M);
    auto K = invL * invM;

    // info_msg("phi = ", phi, " theta = ", theta);

    auto tau = (v + (K * (G + C * dq_s)).at(0,0) + vc2 * pow(dphi_s, 2)) / K.at(0,0);
    return tau;
}



int launch(Json::Value const& jscfg, Json::Value const& fbcfg)
{
    Butterfly bfly;
    bfly.init(jscfg, fbcfg);
    bool stop = false;
    auto stop_handler = [&stop, &bfly]() { stop = true; bfly.stop(); };
    SysSignals::instance().set_sigint_handler(stop_handler);
    SysSignals::instance().set_sigterm_handler(stop_handler);

    auto f_sham = [](BflySignals& signals, FeedbackConfig fbcfg) {
        if (signals.t < 0.1)
            return true;

        if (!signals.ball_found)
            return false;

        auto torque = get_torque(signals, fbcfg);
        signals.torque = clamp(torque, -0.1, 0.1);

        return true;
    };

    bfly.start(f_sham);
    return 0;
}

int main(int argc, char const* argv[])
{
   Arguments args({
//      Argument("-c", "config", "path to json config file", "", ArgumentsCount::One),
//      Argument("-f", "feedback", "path to json feedback config file", "", ArgumentsCount::One)
   });


    
    int status = 0;

    try
    {
//       auto&& m = args.parse(argc, argv);
//       Json::Value const& cfg = json_load(m["config"]);
//       Json::Value const& fbcfg = json_load(m["feedback"]);
     Json::Value const& cfg = json_load("/home/butterfly/students/sham/butterfly-feedback-pub/configs/config.json");
     Json::Value const& fbcfg = json_load("/home/butterfly/Downloads/feedback_parameters.json");
        

        traces::init(json_get(cfg, "traces"));
        launch(cfg, fbcfg);
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

    // return status;
}

