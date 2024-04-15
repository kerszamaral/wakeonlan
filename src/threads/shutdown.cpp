#include "threads/shutdown.hpp"

#include <csignal>
#include "networking/sockets/socket.hpp"

namespace Shutdown
{
    std::function<void(int)> shutdown_handler;

#ifdef _WIN32
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

    void graceful_setup(Threads::Signals &signals)
    {
        shutdown_handler = [&signals](int signal)
        {
            signals.run.store(false);
        };

#ifdef _WIN32
        SetConsoleCtrlHandler(signal_handler, TRUE);
#else
        std::signal(SIGINT, signal_handler);
#endif
    }

    void graceful_shutdown()
    {
        Networking::Sockets::cleanup();
        std::cout << "Shutting down..." << std::endl;
    }
}