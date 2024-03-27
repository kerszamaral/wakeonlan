#include <atomic>
#include <thread>
#include <array>
#include "common/format.hpp"
#include "common/pcinfo.hpp"
#include "common/signal.hpp"
#include "interface/interface.hpp"

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

void setup_signal_handler(std::atomic<bool> &run)
{
    shutdown_handler = [&run](int signal)
    {
        run.store(false);
    };
    std::signal(SIGINT, signal_handler);
}

int main()
{
    auto run = std::atomic<bool>(true);
    auto update = std::atomic<bool>(false);
    setup_signal_handler(run);

    auto pc_map = dummy_pc_map();

    constexpr auto num_subservices = 1;
    std::array<std::thread, num_subservices> subservices;

    constexpr auto interface_service = 0;
    subservices[interface_service] = std::thread(init_interface, std::ref(pc_map), std::ref(run), std::ref(update));

    for (auto &subservice : subservices)
    {
        subservice.join();
    }

    // auto value = Atomic<int>(10);
    // const int num_threads = 2;

    // std::thread threads[num_threads];

    // threads[0] = std::thread(value.with(), test0, 0);
    // threads[1] = std::thread(value.with(), test1, 1);

    return 0;
}
