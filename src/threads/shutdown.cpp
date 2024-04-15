#include "threads/shutdown.hpp"

#include <csignal>
#include "networking/sockets/socket.hpp"

namespace Shutdown
{
    std::function<void(int)> shutdown_handler;
    void signal_handler(int signal)
    {
        shutdown_handler(signal);
    }

    void graceful_setup(Threads::Signals &signals)
    {
        shutdown_handler = [&signals](int signal)
        {
            signals.run.store(false);
        };
        std::signal(SIGINT, signal_handler);
    }

    void graceful_shutdown()
    {
        Networking::Sockets::cleanup();
    }
}