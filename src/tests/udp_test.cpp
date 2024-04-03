#include "tests/udp_test.hpp"
#include "networking/sockets/udp.hpp"

namespace Sockets = Networking::Sockets;
namespace Addr = Networking::Addresses;

int udp_client()
{
    try
    {
        Addr::Address server_addr = Addr::Address("127.0.0.1:8081");
        Sockets::UDP conn = Sockets::UDP(8080);
        std::cout << "UDP connection established" << std::endl;
        conn.send(Networking::Packet(Networking::PacketType::STR, 0, 0, "Hello from client"), server_addr);
        std::cout << "Packet sent" << std::endl;
        auto [packet, addr] = conn.wait_and_receive_packet();
        std::cout << "Packet received: " << packet << std::endl;
        std::cout << "From: " << addr << std::endl;
        conn.close();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int udp_server()
{
    Addr::Address client_addr = Addr::Address("127.0.0.1:8080");
    try
    {
        Sockets::UDP conn = Sockets::UDP(8081);
        std::cout << "UDP connection established" << std::endl;
        auto [packet, addr] = conn.wait_and_receive_packet();
        std::cout << "Packet received: " << packet << std::endl;
        std::cout << "From: " << addr << std::endl;
        conn.send(Networking::Packet(Networking::PacketType::STR, 0, 0, "Hello from server"), client_addr);
        std::cout << "Packet sent" << std::endl;
        conn.close();
    }
    catch (std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int udp_server_client(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        return EXIT_FAILURE;
    }

    const auto &type = args[2];
    if (type == "client")
    {
        return udp_client();
    }
    else if (type == "server")
    {
        return udp_server();
    }
    else
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}