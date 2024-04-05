#include <sstream>
#include <atomic>
#include <thread>
#include <array>
#include <stdexcept>
#include "common/format.hpp"
#include "common/pcinfo.hpp"
#include "common/signal.hpp"
#include "interface/interface.hpp"
#include "networking/sockets/socket.hpp"
#include "tests/tcp_test.hpp"
#include "tests/udp_test.hpp"
#include "tests/broadcast_test.hpp"
#include "common/optional.hpp"

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
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() > 1)
    {
        int ret = -1;
        if (args[1] == "tcp")
        {
            ret = tcp_server_client(args);
        }
        else if (args[1] == "udp")
        {
            ret = udp_server_client(args);
        }
        else if (args[1] == "broad")
        {
            ret = udp_broadcast(args);
        }

        if (ret != 0)
        {
            std::cerr << "Usage: " << args[0] << " <tcp|udp> <client|server>" << std::endl;
            return EXIT_FAILURE;
        }
        return ret;
    }

    auto run = std::atomic<bool>(true);
    auto update = std::atomic<bool>(false);
    auto ended = std::atomic<bool>(false);
    setup_signal_handler(run, ended);

    auto pc_map = dummy_pc_map();

    constexpr auto num_subservices = 1;
    std::array<std::thread, num_subservices> subservices;

    constexpr auto interface_service = 0;
    subservices[interface_service] = std::thread(init_interface, std::ref(pc_map), std::ref(run), std::ref(update));

    for (auto &subservice : subservices)
    {
        subservice.join();
    }
    ended.store(true);

    return EXIT_SUCCESS;
}
