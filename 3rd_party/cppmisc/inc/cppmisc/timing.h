#pragma once
#include <chrono>
#include <time.h>


inline int64_t epoch_usec()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

inline void sleep_usec(int64_t usec)
{
    timespec dt, rem;
    dt.tv_sec = usec / 1000000;
    dt.tv_nsec = (usec % 1000000) * 1000;
    nanosleep(&dt, &rem);
}

inline void sleep_sec(double sec)
{
    int64_t usec = int64_t(sec * 1e+6 + 0.5);
    sleep_usec(usec);
}

inline int64_t sec_to_usec(double sec)
{
    if (sec < 0)
        return int64_t(sec * 1e+6 - 0.5);
    return int64_t(sec * 1e+6 + 0.5);
}

inline double usec_to_sec(int64_t usec)
{
    return usec * 1e-6;
}

class LoopRate
{
private:
    int64_t _t_prev;
    int64_t _interval;

public:
    LoopRate(int64_t interval_usec) : _t_prev(epoch_usec()), _interval(interval_usec) {}
    ~LoopRate() {}

    inline void wait()
    {
        int64_t t = epoch_usec();

        while (t < _t_prev + _interval)
        {
            sleep_usec(_t_prev + _interval - t);
            t = epoch_usec();
        }

        _t_prev = t;
    }
};
