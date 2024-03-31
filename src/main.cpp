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
#include "networking/sockets/tcp.hpp"
#include "networking/sockets/udp.hpp"
#include "networking/addresses/address.hpp"

namespace Sockets = Networking::Sockets;
namespace Addr = Networking::Addresses;

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
        Sockets::cleanup();
    };
    std::signal(SIGINT, signal_handler);
}

int tcp_server()
{
    try
    {
        Sockets::TCP conn = Sockets::TCPServer(8080).wait_for_connection();
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

int tcp_client()
{
    try
    {
        Sockets::TCP conn = Sockets::TCP("127.0.0.1:8080");
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

int tcp_server_client(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "Usage: " << args[0] << " <tcp|udp> <client|server>" << std::endl;
        return EXIT_FAILURE;
    }

    const auto &type = args[2];
    if (type == "server")
    {
        return tcp_server();
    }
    else if (type == "client")
    {
        return tcp_client();
    }
    else
    {
        std::cerr << "Usage: " << args[0] << " <tcp|udp> <client|server>" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int udp_server()
{
    // try
    // {
    //     Sockets::TCP conn = Sockets::TCPServer(8080).wait_for_connection();
    //     std::cout << "TCP connection established" << std::endl;
    //     std::cout << conn << std::endl;
    //     std::istringstream("Hello_from_server") >> conn;
    //     std::cout << "Message sent" << std::endl;
    // }
    // catch (std::runtime_error &e)
    // {
    //     std::cerr << e.what() << std::endl;
    //     return EXIT_FAILURE;
    // }
    return EXIT_SUCCESS;
}

int udp_client()
{
    // try
    // {
    //     Addr::Address addr = Addr::Address("127.0.0.1:8080");
    //     Sockets::UDP conn = Sockets::UDP();
    //     std::cout << "UDP connection established" << std::endl;
    // }
    // catch (std::exception &e)
    // {
    //     std::cerr << e.what() << std::endl;
    //     return EXIT_FAILURE;
    // }
    return EXIT_SUCCESS;
}

int udp_server_client(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        std::cerr << "Usage: " << args[0] << " <tcp|udp> <client|server>" << std::endl;
        return EXIT_FAILURE;
    }

    const auto &type = args[2];
    if (type == "server")
    {
        return udp_server();
    }
    else if (type == "client")
    {
        return udp_client();
    }
    else
    {
        std::cerr << "Usage: " << args[0] << " <tcp|udp> <client|server>" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char const *argv[])
{
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() > 1)
    {
        if (args[1] == "tcp")
        {
            return tcp_server_client(args);
        }
        else if (args[1] == "udp")
        {
            return udp_server_client(args);
        }
        else
        {
            std::cerr << "Usage: " << args[0] << " <tcp|udp> <client|server>" << std::endl;
            return EXIT_FAILURE;
        }
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
