#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>


class TaskQueue
{
private:
    using Task = std::function<void()>;
    std::queue<Task> _tasks;
    std::mutex _task_queue_mtx;
    std::condition_variable _signal;
    std::vector<std::thread> _threads;
    bool _stop;

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
            Task task;

            {
                std::unique_lock<std::mutex> lock(_task_queue_mtx);

                while (_tasks.empty() && !_stop)
                    _signal.wait(lock);

                if (_tasks.empty() && _stop)
                    break;

                task = std::move(_tasks.front());
                _tasks.pop();
            }

            task();
        }
    }

public:
    TaskQueue(int nthreads=1)
    {
        _stop = false;
        for (int i = 0; i < nthreads; ++i)
            _threads.emplace_back(&TaskQueue::loop, this);
    }

    ~TaskQueue()
    {
        stop();
    }

    template <typename ... Args>
    void emplace(Args const& ... args)
    {
        std::unique_lock<std::mutex> lock(_task_queue_mtx);
        _tasks.emplace(args...);
        _signal.notify_all();
    }

    void stop()
    {
        _stop = true;
        _signal.notify_all();

        for (auto& thread : _threads)
        {
            if (!thread.joinable())
                continue;
            thread.join();
        }
    }
};
