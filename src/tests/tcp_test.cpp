#include <iostream>
#include <vector>
#include "networking/sockets/tcp.hpp"

namespace Sockets = Networking::Sockets;
namespace Addr = Networking::Addresses;

Networking::Packet tcp_print_packet(Networking::Packet packet)
{
    std::cout << "Packet received: " << packet << std::endl;
    return packet;
}

int tcp_server()
{
    try
    {
        auto server = Sockets::TCPServer(8080);
        auto conn = server.wait_for_connection();
        conn.and_then([](auto &conn)
                      {std::cout << "TCP connection established with " << conn.getAddress() << std::endl;
                        return conn.receive_packet(); })
            .transform(tcp_print_packet)
            .transform([&conn](auto &&)
                       { return conn->send(Networking::Packet("Hello from server")); })
            .transform([&conn](auto &&)
                       {std::cout << "Packet sent" << std::endl;
                        return conn->close(); });
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
        conn.send(Networking::Packet("Hello from client"))
            .and_then([&conn](auto &&)
                      { std::cout << "Packet sent" << std::endl;
                         return conn.receive_packet(); })
            .transform(tcp_print_packet)
            .transform([&conn](auto &&)
                       { return conn.close(); });
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