#include <thread>
#include <vector>
#include <syncstream>
#include "common/pcinfo.hpp"
#include "threads/signals.hpp"
#include "threads/sighandler.hpp"
#include "networking/addresses/mac.hpp"
#include "subservices/interface/interface.hpp"
#include "subservices/discovery/discovery.hpp"
#include "subservices/management/management.hpp"
#include "subservices/monitoring/monitoring.hpp"
#include "subservices/replication/replication.hpp"
#include "subservices/election/election.hpp"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char const *argv[])
{
    //? Parse command line arguments
#ifdef DEBUG
    std::vector<std::string> args(argv, argv + argc);
    const bool start_as_manager = args.size() > 1 && args[1] == "manager";
#else
    const bool start_as_manager = false;
#endif

    //? Setup safe shutdown handler
    Threads::SigHandler::setup();

#ifdef DEBUG
    const auto hostname = PC::getHostname();
    const auto mac = Networking::Addresses::Mac::FromMachine();
    const auto ip = Networking::Addresses::IPv4::FromMachine();
    {
        /*
            Ideas gotten from

            https://stackoverflow.com/questions/18277304/using-stdcout-in-multiple-threads
            and subsequently
            https://stackoverflow.com/questions/4446484/a-line-based-thread-safe-stdcerr-for-c/53288135#53288135
        */
        std::osyncstream(std::cout) << "DEBUG MODE: Hostname > " << hostname
                                    << " | MAC > " << mac.to_string()
                                    << " | IP > " << ip.to_string()
                                    << " | Starting as > " << (start_as_manager ? "manager" : "client") << "\n"
                                    << std::endl;
    }
#endif

    //? Setup atomic variables
    Threads::Signals::is_manager = start_as_manager;

    //? Setup shared variables
    auto pc_map = PC::atomic_pc_map_t();
    auto new_pcs = PC::new_pcs_queue();
    auto wakeups = PC::wakeups_queue();
    auto sleep_status = PC::sleep_queue();
    auto message_queue = PC::message_queue();

    //? Start subservices
    {
        std::vector<std::jthread> subservices;
        subservices.emplace_back(Subservices::Interface::initialize, std::ref(pc_map), std::ref(wakeups));
        subservices.emplace_back(Subservices::Discovery::initialize, std::ref(new_pcs));
        subservices.emplace_back(Subservices::Management::initialize, std::ref(new_pcs), std::ref(pc_map), std::ref(wakeups), std::ref(sleep_status));
        subservices.emplace_back(Subservices::Monitoring::initialize, std::ref(pc_map), std::ref(sleep_status));
        subservices.emplace_back(Subservices::Replication::initialize, std::ref(pc_map));
        subservices.emplace_back(Subservices::Election::initialize);
    }

    //? Wait for shutdown signal and cleanup
    Threads::SigHandler::teardown();

    return EXIT_SUCCESS;
}
