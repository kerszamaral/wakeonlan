#include "tests/broadcast_test.hpp"
#include "networking/sockets/udp.hpp"

namespace Sockets = Networking::Sockets;
namespace Addr = Networking::Addresses;

int broad_client()
{
    try
    {
        Addr::Port port(8081);
        Sockets::UDP conn = Sockets::UDP(8080);
        std::cout << "Sending UDP broadcast at port " << port << std::endl;
        conn.send_broadcast(Networking::Packet(Networking::PacketType::STR, 0, 0, "Hello from client"), port);
        std::cout << "Packet sent, waiting for a response..." << std::endl;
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

int broad_server()
{
    try
    {
        Sockets::UDP conn = Sockets::UDP(8081);
        std::cout << "UDP server established on port 8081" << std::endl;
        auto [packet, addr] = conn.wait_and_receive_packet();
        std::cout << "Packet received: " << packet << std::endl;
        std::cout << "From: " << addr << std::endl;
        conn.send(Networking::Packet(Networking::PacketType::STR, 0, 0, "Hello from server"), addr);
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

int udp_broadcast(const std::vector<std::string> &args)
{
    if (args.size() < 2)
    {
        return EXIT_FAILURE;
    }

    const auto &type = args[2];
    if (type == "client")
    {
        return broad_client();
    }
    else if (type == "server")
    {
        return broad_server();
    }
    else
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}