#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <functional>
#include <iostream>


class ThreadPool
{
private:
    using Task = std::function<void()>;

    std::vector<std::thread> _threads;
    std::queue<Task> _tasks;
    std::mutex _task_queue_mtx;

    inline Task pop_next()
    {
        std::lock_guard<std::mutex> lock(_task_queue_mtx);
        if (_tasks.empty())
            return Task();
        Task task = std::move(_tasks.front());
        _tasks.pop();
        return task;
    }

    void loop()
    {
        while (true)
        {
            Task f = pop_next();
            if (!f)
                break;
            f();
        }
    }

public:
    ThreadPool()
    {
    }

    template <typename ... Args>
    void emplace(Args const& ... args)
    {
        _tasks.emplace(args...);
    }

    void launch(int nthreads=-1)
    {
        if (nthreads < 0)
            nthreads = std::thread::hardware_concurrency();

        auto f = [this]() { loop(); };
        for (int i = 0; i < nthreads; ++ i)
        {
            _threads.emplace_back(f);
        }

        for (int i = 0; i < nthreads; ++ i)
        {
            _threads[i].join();
        }

        _threads.clear();
    }
};
