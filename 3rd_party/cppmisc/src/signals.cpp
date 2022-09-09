#include <cppmisc/signals.h>
#include <cppmisc/traces.h>
#include <signal.h>


void SysSignals::set_sigint_handler(sighandler_t handler)
{
    _sigint_handler = std::move(handler);
    signal(SIGINT, &SysSignals::handle);
}

void SysSignals::set_sigterm_handler(sighandler_t handler)
{
    _sigterm_handler = std::move(handler);
    signal(SIGTERM, &SysSignals::handle);
}

void SysSignals::set_sighup_handler(sighandler_t handler)
{
    _sighup_handler = std::move(handler);
    signal(SIGHUP, &SysSignals::handle);
}

void SysSignals::handle(int sig)
{
    SysSignals& inst = SysSignals::instance();

    switch (sig)
    {
    case SIGTERM:
        if (inst._sigterm_handler)
            inst._sigterm_handler();
        break;
    case SIGINT:
        if (inst._sigint_handler)
            inst._sigint_handler();
        break;
    case SIGHUP:
        if (inst._sighup_handler)
            inst._sighup_handler();
        break;
    }
}

SysSignals& SysSignals::instance()
{
    static SysSignals signals;
    return signals;
}

SysSignals::SysSignals()
{
}

SysSignals::~SysSignals()
{
}
