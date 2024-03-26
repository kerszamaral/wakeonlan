#include <atomic>
#include <thread>
#include "common/format.hpp"
#include "common/constants.hpp"
#include "common/signal.hpp"
#include "interface/interface.hpp"

pc_map_t dummy_pc_map()
{
    pc_map_t pc_map = std::map<MacAddress, pc_info>();
    for (int i = 0; i < 20; i++)
    {
        pc_info pc = pc_info();
        sprintf(pc.name, "TEST%d", i);
        std::string mac_addr = std::string(fmt::string_format("%02x:%02x:%02x:%02x:%02x:%02x", i, i, i, i, i, i));
        MacAddress mac = MacAddress(mac_addr);

        pc_map.insert(std::make_pair(mac, pc));
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
    std::vector<std::thread> subservices(num_subservices);

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
