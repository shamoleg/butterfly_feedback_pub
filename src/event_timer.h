#include <functional>
#include <thread>
#include <memory>



typedef std::function<int(void)> EventTimerHandler;

class EventTimer
{
private:
    enum state_t { active, stopping, stopped };

    EventTimerHandler               user_handler;
    int64_t                         period_usec;
    state_t                         state;
    std::unique_ptr<std::thread>    caller;

    void handler_loop()
    {
        auto t = epoch_usec();
        auto t0 = t;

        try
        {
            while (state == active)
            {
                int status = user_handler();
                if (status)
                    break;

                t = epoch_usec();
                auto t1 = t0 + period_usec;

                while (t < t1)
                {
                    sleep_usec(t1 - t);
                    t = epoch_usec();
                }

                t0 = epoch_usec();
            }
        }
        catch (std::exception& e)
        {
            err_msg(e.what());
        }

        this->caller->detach();
        state = stopped;
    }

public:
    EventTimer(int64_t period_usec, EventTimerHandler const& handler) :
        user_handler(handler),
        period_usec(period_usec),
        state(stopped)
    {}

    ~EventTimer()
    {
        stop();
    }

    inline void run()
    {
        state = active;
        caller.reset(new std::thread([this]() { this->handler_loop(); }));
    }

    inline void wait()
    {
        if (caller && caller->joinable())
            caller->join();
    }

    inline void stop()
    {
        if (state == stopped)
            return;

        state = stopping;

        if (caller && caller->joinable())
            caller->join();
    }

    inline bool is_run()
    {
        return state == active;
    }
};

