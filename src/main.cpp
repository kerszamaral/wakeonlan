#include <sstream>
#include <atomic>
#include <thread>
#include <array>
#include <stdexcept>
#include "common/format.hpp"
#include "common/pcinfo.hpp"
#include "common/shutdown.hpp"
#include "interface/interface.hpp"
#include "networking/sockets/socket.hpp"
#include "discovery/discovery.hpp"

pc_map_t dummy_pc_map()
{
    pc_map_t pc_map = pc_map_t();
    for (int i = 0; i < 20; i++)
    {
        auto hostname = fmt::format("TEST%d", i);
        auto mac_addr = fmt::format("%02x:%02x:%02x:%02x:%02x:%02x", i, i, i, i, i, i);
        auto ipv4_addr = fmt::format("192.168.1.%d", i);
        auto status = PC_STATUS::AWAKE;
        auto pc = PCInfo(hostname, mac_addr, ipv4_addr, status);
        pc_map.emplace(hostname, pc);
    }
    return pc_map;
}

void setup_signal_handler(std::atomic<bool> &run, std::atomic<bool> &ended)
{
    shutdown_handler = [&run, &ended](int signal)
    {
        run.store(false);
        while (!ended.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        Networking::Sockets::cleanup();
    };
    std::signal(SIGINT, signal_handler);
}

int main(int argc, char const *argv[])
{
    //? Parse command line arguments
    std::vector<std::string> args(argv, argv + argc);
    bool start_as_manager = args.size() > 1 && args[1] == "manager";

    //? Setup atomic variables
    auto is_manager = std::atomic<bool>(start_as_manager);
    auto run = std::atomic<bool>(true);
    auto update = std::atomic<bool>(false);
    auto ended = std::atomic<bool>(false);
    auto manager_found = std::atomic<bool>(false);

    //? Setup signal handler
    setup_signal_handler(run, ended);

    auto pc_map = dummy_pc_map();

    //? Start subservices
    constexpr auto num_subservices = 2; //! Don't forget to update this number
    std::array<std::thread, num_subservices> subservices;

    constexpr auto interface_service = 0;
    subservices[interface_service] = std::thread(init_interface, std::ref(pc_map), std::ref(run), std::ref(update));

    constexpr auto discovery_service = 1;
    subservices[discovery_service] = std::thread(init_discovery, std::ref(is_manager), std::ref(run), std::ref(update), std::ref(manager_found));

    for (auto &subservice : subservices)
    {
        subservice.join();
    }
    ended.store(true);

    return EXIT_SUCCESS;
}
