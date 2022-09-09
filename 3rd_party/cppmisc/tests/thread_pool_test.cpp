#include <iostream>
#include <cppmisc/thread_pool.h>
#include <cppmisc/files.h>
#include <cppmisc/timing.h>
#include <atomic>
#include <assert.h>


int64_t sum(char const* data, int sz)
{
    int64_t val = 0;
    for (int i = 0; i < sz; ++i)
    {
        val += data[i];
    }
    return val;
}

int64_t process_v1(std::vector<char> const& data)
{
    return sum(&data[0], data.size());
}

int64_t process_v2(std::vector<char> const& data)
{
    int64_t chunk_sz = 8192;
    int N = data.size() / chunk_sz;
    std::atomic<std::int64_t> result {0};

    ThreadPool tp;
    for (int i = 0; i < N; ++ i)
    {
        auto f = [&data,i,N,&result]() {
            auto a = data.size() * i / N;
            auto b = data.size() * (i + 1) / N;
            int64_t val = sum(&data[a], b - a);
            result += val;
        };
        tp.emplace(f);
    }

    tp.launch(8);
    return result;
}

int main(int argc, char const* argv[])
{
    std::vector<char> data(32*1024*1024);

    for (int i = 0; i < (int)data.size(); ++ i)
    {
        data[i] = rand() % 256;
    }

    auto t1 = epoch_usec();
    int64_t v1 = process_v1(data);
    auto t2 = epoch_usec();
    int64_t v2 = process_v2(data);
    auto t3 = epoch_usec();
    assert(v1 == v2);
    auto dt1 = t2 - t1;
    auto dt2 = t3 - t2;
    std::cout << "values: " << v1 << " " << v2 << std::endl;
    std::cout << "multithreading factor is " << 1. * dt1 / dt2 << 
        "; single thread: " << dt1 << "usec" <<
        "; 4 threads: " << dt2 << std::endl;
    return 0;
}
