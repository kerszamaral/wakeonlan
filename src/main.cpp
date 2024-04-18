#include <thread>
#include <vector>
#include "common/pcinfo.hpp"
#include "threads/signals.hpp"
#include "threads/sighandler.hpp"
#include "networking/addresses/mac.hpp"
#include "subservices/interface/interface.hpp"
#include "subservices/discovery/discovery.hpp"
#include "subservices/management/management.hpp"
#include "subservices/monitoring/monitoring.hpp"

int main(int argc, char const *argv[])
{
    //? Parse command line arguments
    std::vector<std::string> args(argv, argv + argc);
    const bool start_as_manager = args.size() > 1 && args[1] == "manager";

    //? Setup safe shutdown handler
    Threads::SigHandler::setup();

#ifdef DEBUG
    const auto hostname = PC::getHostname();
    const auto mac = Networking::Addresses::Mac::FromMachine().value();
    std::cout << "DEBUG MODE: Hostname > " << hostname
              << " | MAC > " << mac.to_string()
              << " | Starting as > " << (start_as_manager ? "manager" : "client") << "\n"
              << std::endl;
#endif

    //? Setup atomic variables
    Threads::Signals::is_manager = start_as_manager;

    //? Setup shared variables
    auto pc_map = PC::atomic_pc_map_t();
    auto new_pcs = PC::new_pcs_queue();
    auto wakeups = PC::wakeups_queue();
    auto sleep_status = PC::sleep_queue();

    //? Start subservices
    {
        std::vector<std::jthread> subservices;
        subservices.emplace_back(Subservices::Interface::initialize, std::ref(pc_map), std::ref(wakeups));
        subservices.emplace_back(Subservices::Discovery::initialize, std::ref(new_pcs));
        subservices.emplace_back(Subservices::Management::initialize, std::ref(new_pcs), std::ref(pc_map), std::ref(wakeups));
        subservices.emplace_back(Subservices::Monitoring::initialize, std::ref(pc_map), std::ref(sleep_status));
    }

    //? Wait for shutdown signal and cleanup
    Threads::SigHandler::teardown();

    return EXIT_SUCCESS;
}
