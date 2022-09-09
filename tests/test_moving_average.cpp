#include <cppmisc/traces.h>
#include <cppmisc/timing.h>
#include "../src/math_helpers.h"
#include "../src/moving_average.h"


void test1()
{
	SigHistory history(10);
	for (int i = 1; i <= 4; ++i)
		history.push({i, double(i)});
	assert(history.size() == 4);
	for (int i = 5; i <= 10; ++i)
		history.push({i, double(i)});
	assert(history.size() == 10);
	for (int i = 11; i <= 13; ++i)
		history.push({i, double(i)});
	assert(history.size() == 10);
	history.clear_old(0);
	assert(history.size() == 10);
	history.clear_old(9);
	assert(history.size() == 4);
	history.clear_old(5);
	assert(history.size() == 4);
	history.clear_old(15);
	assert(history.size() == 0);
	for (int i = 1; i <= 13; ++i)
		history.push({i, double(i)});
	assert(history.size() == 10);
}

void test2()
{
	SigHistory history(10);

	for (auto const& e : history)
		assert(false);

	history.push({0, 0.0});
	int n = 0;
	for (auto const& e : history)
	{
		++ n;
		assert(e.ts == 0);
	}
	assert(n == 1);

	std::vector<TimedSignal> src = {
		{1, 1.0},
		{2, 2.0},
		{3, 3.0},
		{4, 4.0},
		{5, 5.0},
		{6, 6.0},
		{7, 7.0},
		{8, 8.0},
		{9, 9.0},
		{10, 10.0},
		{11, 11.0},
		{12, 12.0},
	};

	for (auto const& e : src)
		history.push(e);

	auto i1 = history.begin();
	auto i2 = src.rbegin();
	n = 0;

	while (i1 != history.end())
	{
		assert(i1->ts == i2->ts);
		assert(i1->value == i2->value);
		++ i1;
		++ i2;
		++ n;
	}

	assert(n == 10);
}

void test3()
{
	SigHistory history(10);

	std::vector<TimedSignal> src = {
		{1, 1.0},
		{2, 2.0},
		{3, 3.0},
		{4, 4.0},
		{5, 5.0},
		{6, 6.0},
		{7, 7.0},
		{8, 8.0},
		{9, 9.0},
		{10, 10.0},
		{11, 11.0},
		{12, 12.0},
	};

	for (auto const& e : src)
		history.push(e);

	auto i1 = history.rbegin();
	auto i2 = src.begin() + 2;
	int n = 0;

	while (i1 != history.rend())
	{
		dbg_msg(i1->ts);
		assert(i1->ts == i2->ts);
		assert(i1->value == i2->value);
		++ i1;
		++ i2;
		++ n;
	}
}

void test4()
{
	double step = 1e-3;
	MovingAverage average(step, 1.);
	for (int i = 0; i < 1000; ++ i)
	{
		double t = i * step;
		double u = sin(t);
		average.update(sec_to_usec(t), u);
	}

	dbg_msg("value: ", average.value(), " ", cos(0.) - cos(1.));
}

int main(int argc, char const* argv[])
{
	test1();
	test2();
	test3();
	test4();
	return 0;
}
