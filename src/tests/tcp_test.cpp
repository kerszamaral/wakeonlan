#include <iostream>
#include <vector>
#include "networking/sockets/tcp.hpp"

namespace Sockets = Networking::Sockets;
namespace Addr = Networking::Addresses;

int tcp_server()
{
    try
    {
        auto server = Sockets::TCPServer(8080);
        auto conn = server.wait_for_connection();
        std::cout << "TCP connection established with " << conn.getAddress() << std::endl;
        auto packet = conn.receive_packet();
        std::cout << "Message received: " << packet.getPayload() << std::endl;
        conn.send(Networking::Packet(Networking::PacketType::STR, 0, 0, "Hello from server"));
        std::cout << "Message sent" << std::endl;
        conn.close();
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
        std::cout << "TCP connection established with " << conn.getAddress() << std::endl;
        conn.send(Networking::Packet(Networking::PacketType::STR, 0, 0, "Hello from client"));
        std::cout << "Message sent" << std::endl;
        auto packet = conn.receive_packet();
        std::cout << "Message received: " << packet.getPayload() << std::endl;
        conn.close();
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
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}