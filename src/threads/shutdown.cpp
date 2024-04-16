#include "threads/shutdown.hpp"

#include <csignal>
#include "common/platform.hpp"
#include "networking/sockets/socket.hpp"
#include "threads/signals.hpp"

namespace Shutdown
{
    std::function<void(int)> shutdown_handler;

#ifdef OS_WIN
    BOOL WINAPI signal_handler(_In_ DWORD dwCtrlType)
    {
        switch (dwCtrlType)
        {
        case CTRL_C_EVENT:
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

    void graceful_setup()
    {
        shutdown_handler = [](int signal)
        {
            Threads::Signals::run = false;
        };

#ifdef OS_WIN
        SetConsoleCtrlHandler(signal_handler, TRUE);
#else
        std::signal(SIGINT, signal_handler);
#endif
    }

    void graceful_shutdown()
    {
        Networking::Sockets::cleanup();
#ifdef DEBUG
        std::cout << "Shutting down..." << std::endl;
#endif
    }
}