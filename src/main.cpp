#include <thread>
#include <vector>
#include "common/pcinfo.hpp"
#include "threads/signals.hpp"
#include "threads/shutdown.hpp"
#include "interface/interface.hpp"
#include "discovery/discovery.hpp"
#include "management/management.hpp"
#include "networking/addresses/mac.hpp"

int main(int argc, char const *argv[])
{
#ifdef DEBUG
    std::cout << "DEBUG MODE" << std::endl;
    const auto hostname = PC::getHostname();
    const auto mac = Networking::Addresses::Mac::FromMachine().value();
    std::cout << "Our hostname: " << hostname << " | Our MAC: " << mac.to_string() << std::endl;
#endif

    //? Parse command line arguments
    std::vector<std::string> args(argv, argv + argc);
    const bool start_as_manager = args.size() > 1 && args[1] == "manager";

    //? Setup atomic variables
    Threads::Signals::is_manager = start_as_manager;

    //? Setup safe shutdown handler
    Shutdown::graceful_setup();

    //? Setup shared variables
    auto pc_map = PC::atomic_pc_map_t();
    auto new_pcs = PC::new_pcs_queue();
    auto wakeups = PC::wakeups_queue();

    //? Start subservices
    {
        std::vector<std::jthread> subservices;
        subservices.emplace_back(init_interface, std::ref(pc_map), std::ref(wakeups));
        subservices.emplace_back(init_discovery, std::ref(new_pcs));
        subservices.emplace_back(init_management, std::ref(new_pcs), std::ref(pc_map), std::ref(wakeups));
    }

    Shutdown::graceful_shutdown();
    return EXIT_SUCCESS;
}
