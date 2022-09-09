#pragma once

#include <cmath>

static const double _PI_2 = acos(0.0);
static const double _PI = 2 * _PI_2;
static const double _2_PI = 2 * _PI;
static const double _4_PI = 4 * _PI;

template <typename T>
inline T clamp(T const& val, T const& minval, T const& maxval)
{
	return val < minval ? minval : 
		val > maxval ? maxval : val;
}

template <typename T>
inline bool in_diap(T const& val, T const& minval, T const& maxval)
{
    return val >= minval && val <= maxval;
}

template <typename T>
inline T square(T const& x)
{
	return x * x;
}

template <typename T>
inline T sign(T const& x)
{
	return (T)(x > 0) * (T)2 - (T)1;
}

inline double modulo(double a, double b)
{
	int n = (int)floor(a / b);
	return a - n * b;
}

inline double excess(double a, double b)
{
    long n = lround(a / b);
    return a - n * b;
}

inline double triangle(double t, double T)
{
	return fabs(modulo(t / (T * 0.25), 4.) - 2.) - 1.;
}

inline double random(double a, double b)
{
	double r = std::rand();
	return a + (b - a) * (r / RAND_MAX);
}
