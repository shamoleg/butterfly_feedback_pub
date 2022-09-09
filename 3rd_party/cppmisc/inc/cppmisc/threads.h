#include <pthread.h>


inline bool set_thread_rt_priotiy(pthread_t thread, int priotity)
{
    if (thread == pthread_t(-1))
        thread = pthread_self();

    int policy = SCHED_RR; // SCHED_FIFO;
    sched_param param;
    param.sched_priority = priotity;
    return pthread_setschedparam(thread, policy, &param) == 0;
}

inline bool clear_thread_priority(pthread_t thread)
{
    if (thread == pthread_t(-1))
        thread = pthread_self();

    int policy = SCHED_OTHER;
    sched_param param;
    param.sched_priority = 0;
    return pthread_setschedparam(thread, policy, &param) == 0;
}
