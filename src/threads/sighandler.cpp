#include "threads/sighandler.hpp"

#include <functional>
#include <csignal>
#include "common/platform.hpp"
#include "networking/sockets/socket.hpp"
#include "threads/signals.hpp"
#include <syncstream>

namespace Threads::SigHandler
{
    std::function<void(int)> shutdown_handler = [](int signal __attribute__((unused)))
    {
        Threads::Signals::run = false;
        Threads::Signals::run.notify_all();
        Threads::Signals::update = true;
        Threads::Signals::update.notify_all();
        Threads::Signals::current_manager = 0;
        Threads::Signals::current_manager.notify_all();
    };

#ifdef OS_WIN
    BOOL WINAPI signal_handler(_In_ DWORD dwCtrlType)
    {
        switch (dwCtrlType)
        {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
            shutdown_handler(dwCtrlType);
            break;
        default:
            return FALSE;
        }

        return TRUE;
    }
#else
    void signal_handler(int signal)
    {
        shutdown_handler(signal);
    }
#endif

    void setup()
    {
        Networking::Sockets::initialize();

#ifdef OS_WIN
        SetConsoleCtrlHandler(signal_handler, TRUE);
#else
        std::signal(SIGINT, signal_handler);
#endif
    }

    void teardown()
    {
        Networking::Sockets::cleanup();
#ifdef DEBUG
        std::osyncstream(std::cout) << "Shutting down..." << std::endl;
#endif
    }

    void run_handler(int signal)
    {
        shutdown_handler(signal);
    }
}