#pragma once
#include <vector>
#include <memory>


class basic_spline
{
public:
	virtual double val(double x, int der=0) const = 0;
	virtual double der(double x) const = 0;
	virtual double der2(double x) const = 0;
	virtual double der3(double x) const = 0;
};

class spline
{
private:
	enum extrapolation
	{
		none,
		periodic,
		end_value
	};

	std::unique_ptr<basic_spline> 	s;
	extrapolation  					extr;
	double  						minval, maxval, period;

	double fix_arg(double x) const;

public:
	spline(
		unsigned int degree, 
		std::vector<double> const& knots, 
		std::vector<double> const& coefs,
		std::string const& extrapolation = "none" // periodic, none, end-value
	);
	double val(double x, int der=0) const;
	double der(double x) const;
	double der2(double x) const;
	double der3(double x) const;

	inline double operator () (double x, int der=0) const
	{
		return val(x, der);
	}
};
