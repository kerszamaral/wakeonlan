#include <sstream>
#include <atomic>
#include <thread>
#include <vector>
#include <stdexcept>
#include "common/pcinfo.hpp"
#include "common/shutdown.hpp"
#include "interface/interface.hpp"
#include "networking/sockets/socket.hpp"
#include "discovery/discovery.hpp"
#include "management/management.hpp"
#include "threads/prodcosum.hpp"
#include "threads/atomic.hpp"
#include "threads/signals.hpp"

void setup_signal_handler(Threads::Signals &signals)
{
    shutdown_handler = [&signals](int signal)
    {
#ifndef DEBUG
        std::cout << "Received signal " << signal << std::endl;
#endif
        constexpr const auto WAIT_DELAY = std::chrono::milliseconds(100);
        signals.run.store(false);
        while (!signals.ended.load())
        {
#ifndef DEBUG
            std::cout << "Waiting for subservices to end" << std::endl;
#endif
            std::this_thread::sleep_for(WAIT_DELAY);
        }
#ifndef DEBUG
        std::cout << "Shutting down" << std::endl;
#endif
        Networking::Sockets::cleanup();
        std::exit(EXIT_SUCCESS);
    };
    std::signal(SIGINT, signal_handler);
}

int main(int argc, char const *argv[])
{
    //? Parse command line arguments
    std::vector<std::string> args(argv, argv + argc);
    const bool start_as_manager = args.size() > 1 && args[1] == "manager";

    //? Setup atomic variables
    auto signals = Threads::Signals(start_as_manager);

    //? Setup safe shutdown handler
    setup_signal_handler(signals);

    //? Setup shared variables
    auto pc_map = Threads::Atomic<pc_map_t>();
    auto new_pcs = Threads::ProdCosum<PCInfo>();
    auto wakeups = Threads::ProdCosum<hostname_t>();

    //? Start subservices
    {
        std::vector<std::jthread> subservices;
        subservices.emplace_back(init_interface, std::ref(pc_map), std::ref(signals), std::ref(wakeups));
        subservices.emplace_back(init_discovery, std::ref(new_pcs), std::ref(signals));
        subservices.emplace_back(init_management, std::ref(new_pcs), std::ref(pc_map), std::ref(wakeups), std::ref(signals));
    }
#ifndef DEBUG
    std::cout << "Main thread shutting down" << std::endl;
#endif
    signals.ended.store(true);

    return EXIT_SUCCESS;
}
