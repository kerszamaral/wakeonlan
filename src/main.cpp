#include <sstream>
#include <atomic>
#include <thread>
#include <array>
#include <stdexcept>
#include "common/format.hpp"
#include "common/pcinfo.hpp"
#include "common/signal.hpp"
#include "interface/interface.hpp"
#include "networking/socket.hpp"
#include "networking/listener.hpp"
#include "networking/tcp.hpp"

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
        Socket::cleanup();
    };
    std::signal(SIGINT, signal_handler);
}

int test_server()
{
    try
    {
        TCP conn = PortListener(8080).waitForConnection();
        std::cout << "TCP connection established" << std::endl;
        std::cout << conn << std::endl;
        std::istringstream("Hello_from_server") >> conn;
        std::cout << "Message sent" << std::endl;
    }
    catch (std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int test_client()
{
    try
    {
        TCP conn("127.0.0.1", 8080);
        std::cout << "TCP  connection established" << std::endl;
        std::istringstream("Hello_from_client") >> conn;
        std::cout << "Message sent" << std::endl;
        std::cout << conn << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int test_server_client(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <server|client>" << std::endl;
        return EXIT_FAILURE;
    }

    auto type = std::string(argv[1]);
    if (type == "server")
    {
        return test_server();
    }
    else if (type == "client")
    {
        return test_client();
    }
    else
    {
        std::cerr << "Usage: " << argv[0] << " <server|client>" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char const *argv[])
{
    if (argc > 1)
    {
        return test_server_client(argc, argv);
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
