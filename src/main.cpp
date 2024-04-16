#include <thread>
#include <vector>
#include "common/pcinfo.hpp"
#include "threads/atomic.hpp"
#include "threads/signals.hpp"
#include "threads/shutdown.hpp"
#include "threads/prodcosum.hpp"
#include "networking/macaddr.hpp"
#include "interface/interface.hpp"
#include "discovery/discovery.hpp"
#include "management/management.hpp"

int main(int argc, char const *argv[])
{
#ifdef DEBUG
    std::cout << "DEBUG MODE" << std::endl;
    const auto hostname = PCInfo::getMachineName();
    const auto mac = Networking::MacAddress::getMacAddr().value();
    std::cout << "Our hostname: " << hostname << " | Our MAC: " << mac.to_string() << std::endl;
#endif

    //? Parse command line arguments
    std::vector<std::string> args(argv, argv + argc);
    const bool start_as_manager = args.size() > 1 && args[1] == "manager";

    //? Setup atomic variables
    Threads::Signals::is_manager.store(start_as_manager);

    //? Setup safe shutdown handler
    Shutdown::graceful_setup();

    //? Setup shared variables
    auto pc_map = Threads::Atomic<pc_map_t>();
    auto new_pcs = Threads::ProdCosum<PCInfo>();
    auto wakeups = Threads::ProdCosum<hostname_t>();

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
