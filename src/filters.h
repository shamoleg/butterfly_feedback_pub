#pragma once

#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <math.h>
#include <stdint.h>
#include <assert.h>
#include <cppmisc/timing.h>

#include "matrix.h"
#include "math_helpers.h"


//
//      As + B
// -------------------
// (Cs + 1) * (Ds + 1)
//
class TransFunc
{
private:
    double      A11, A12, A21, A22;
    double      B1, B2;
    int64_t     min_dt;

    double      y0, z0, x0;
    int64_t     t0;
    bool        bfirstrun;

    inline void set_initial_values(int64_t const& now, double x)
    {
        double d = A11 * A22 - A21 * A12;
        y0 = -1. * x * (A22 * B1 - A12 * B2) / d;
        z0 = -1. * x * (-A21 * B1 + A11 * B2) / d;
        t0 = now;
        x0 = x;
    }

public:
    TransFunc(double A, double B, double C, double D)
    {
        init(A,B,C,D);
    }

    TransFunc()
    {
        init(0, 1, 1, 1);
    }

    inline void init(double const& A, double const& B, double const& C, double const& D)
    {
        assert(C != 0.0);
        assert(D != 0.0);

        A11 = -(C+D) / (C*D);
        A12 = 1. / (C*D);
        A21 = -1.;
        A22 = 0.;
        B1 = A / (C*D);
        B2 = B;
        bfirstrun = true;
        min_dt = int64_t(std::min(C, D) * 1e+6 / 2);
    }

    inline void reset()
    {
        bfirstrun = true;
    }

    inline double process(int64_t const& now, double const& x)
    {
        if (bfirstrun)
        {
            bfirstrun = false;
            set_initial_values(now, x);
        }

        if (now - t0 > min_dt)
        {
            process((t0 + now) / 2, (x + x0) / 2);
        }

        double dt = 1e-6 * (now - t0);
        double dy = A11 * y0 + A12 * z0 + B1 * x;
        double dz = A21 * y0 + A22 * z0 + B2 * x;

        double y1 = y0 + dy * dt;
        double z1 = z0 + dz * dt;

        y0 = y1;
        z0 = z1;
        t0 = now;
        x0 = x;

        return y0;
    }
};

//
// src: angle % 2 PI
// dst: angle
//
class ProlongateAngFilt
{
private:
    double x0;
    int n_revs;

public:
    ProlongateAngFilt()
    {
        n_revs = 0;
        x0 = 0;
    }

    inline void reset()
    {
        n_revs = 0;
        x0 = 0;
    }

    inline double process(double const& x)
    {
        bool const b1 = (x - x0) < - M_PI;
        bool const b2 = (x - x0) > M_PI;

        x0 = x;
        n_revs += b1;
        n_revs -= b2;

        return x + n_revs * 2 * M_PI;
    }
};

//
// delay signal
//
class DelayFilt
{
private:
    struct sig_t {
        int64_t t;
        double x;
        sig_t() : t(0), x(0.0) {}
    };

    std::vector<sig_t>  arr;
    int64_t             delay;
    int                 pointer;

public:
    DelayFilt()
    {
        init(0, 0);
    }

    void init(int64_t const& delay_usec, int buf_size = 32)
    {
        delay = delay_usec;
        pointer = 0;
        arr.resize(buf_size);
    }

    DelayFilt(int64_t const& delay_usec, int buf_size = 32)
    {
        init(delay_usec, buf_size);
    }

    inline double process(int64_t const& t_usec, double const& x)
    {
        int last = modulo(pointer + 1, arr.size());
        int delayed = last;

        for (int i = pointer; i != last; i = modulo(i - 1, arr.size()))
        {
            if (arr[i].t + delay < t_usec)
            {
                delayed = i;
                break;
            }
        }

        double res = arr[delayed].x;

        pointer = modulo(pointer + 1, arr.size());
        arr[pointer].t = t_usec;
        arr[pointer].x = x;

        return res;
    }
};

//
// simple diff:
//  (x - x_prev) / (t - t_prev)
//
class EulerDiff
{
private:
    double x0;
    double dx;
    int64_t t0_usec;
    double epsilon;

public:
    EulerDiff(double epsilon = 1e-8) : 
        epsilon(epsilon)
    {
        x0 = 0;
        t0_usec = 0;
    }

    inline double process(int64_t const& t_usec, double const& x)
    {
        if (fabs(x - x0) > epsilon)
        {
            dx = (x - x0) * 1e+6 / (t_usec - t0_usec);
            x0 = x;
            t0_usec = t_usec;
        }

        return dx;      
    }
};

class Integrator
{
private:
    double x_prev;
    int64_t t_prev;
    double sum;

public:
    Integrator()
    {
        t_prev = -1;
        x_prev = 0.0;
        sum = 0.0;
    }

    void update(int64_t const& t, double const& x)
    {
        if (t_prev < 0)
        {
            t_prev = t;
            x_prev = x;
            sum = 0.0;
        }

        sum += (t - t_prev) * 1e-6 * (x + x_prev) / 2;
        t_prev = t;
        x_prev = x;
    }

    double value() const
    {
        return sum;
    }
};


/*
 * Luenberger observer of the system
 *  theta'' = J u
 * with theta measured
 */
class VelocityObserver
{
private:
    Vec2 x_;
    Mat2x1 L;
    Mat2x2 A;
    Mat1x2 C;
    Mat2x1 B;
    Mat2x2 K;
    int64_t t_prev;
    bool first;

public:
    VelocityObserver(double J)
    {
        x_ = {0, 0};
        A = {
            0, 1,
            0, 0
        };
        B = {
            0,
            1. / J
        };
        C = {1, 0};
        // L = 0.01 * C.t();
        L = {10, 200};
        K = A - L * C;
        first = true;
    }

    void update(int64_t t_usec, double theta, double u)
    {
        if (first)
        {
            first = false;
            x_ = {theta, 0};
            t_prev = t_usec;
        }

        double y = theta;
        double y_ = (C * x_)(0,0);
        Mat2x1 dx_ = A * x_  + B * u + L * (y - y_);
        double dt = usec_to_sec(t_usec - t_prev);
        t_prev = t_usec;
        x_ = x_ + dx_ * dt;
    }

    inline double value() const
    {
        return x_(1);
    }
};

