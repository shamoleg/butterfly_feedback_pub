#include <atomic>
#include <cppmisc/task_queue.h>
#include <cppmisc/misc.h>
#include <cppmisc/timing.h>
#include <iostream>
#include <assert.h>


void test_conseq()
{
    int val = 0;
    int N = 1000;
    TaskQueue tasks;

    for (int i = 0; i < N; ++ i)
    {
        auto f = [&val,i]() {
            assert(val == i);
            val += 1;
            unused(i);
        };
        tasks.emplace(f);
    }

    tasks.stop();
    assert(val == N);
}

void test_multithreading()
{
    int ntasks = 23454;
    int nthreads = 6;
    TaskQueue tasks(nthreads);
    std::atomic_int sum {0};

    for (int i = 0; i < ntasks; ++ i)
    {
        auto f = [&sum,i]() { sleep_usec(10); sum += i; sleep_usec(10); };
        tasks.emplace(f);
    }

    tasks.stop();
    assert(sum == ntasks * (ntasks - 1) / 2);
}

void test_perf()
{
    int N = 1000;
    TaskQueue tasks;
    auto f = []() { sleep_usec(1000); };

    // test 1
    auto dt1 = epoch_usec();
    for (int i = 0; i < N; ++ i)
    {
        tasks.emplace(f);
    }
    tasks.stop();
    dt1 = epoch_usec() - dt1;

    // test 2
    auto dt2 = epoch_usec();
    for (int i = 0; i < N; ++ i)
    {
        f();
    }
    dt2 = epoch_usec() - dt2;
    std::cout << "approx equal: " << dt1 << ", " << dt2 << std::endl;
}

int main(int argc, char const* argv[])
{
    test_perf();
    test_conseq();
    test_multithreading();
    return 0;
}
