#include "tests/udp_test.hpp"
#include "networking/sockets/udp.hpp"

namespace Sockets = Networking::Sockets;
namespace Addr = Networking::Addresses;

Addr::Address udp_print_packet(const std::pair<Networking::Packet, Addr::Address> &packet)
{
    std::cout << "Packet received: " << packet.first << std::endl;
    std::cout << "From: " << packet.second << std::endl;
    return packet.second;
}

int udp_client()
{
    try
    {
        Addr::Address server_addr = Addr::Address("127.0.0.1:8081");
        Sockets::UDP conn = Sockets::UDP(8080);
        std::cout << "UDP connection established" << std::endl;
        conn.send(Networking::Packet("Hello from client"), server_addr)
            .and_then([](auto &&conn)
                      {std::cout << "Packet sent" << std::endl;
                        return conn.get().wait_and_receive_packet(); })
            .transform(udp_print_packet)
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

int udp_server()
{
    Addr::Address client_addr = Addr::Address("127.0.0.1:8080");
    try
    {
        Sockets::UDP conn = Sockets::UDP(8081);
        std::cout << "UDP connection established" << std::endl;
        conn.wait_and_receive_packet()
            .transform(udp_print_packet)
            .and_then([&conn](const Addr::Address &addr)
                      { return conn.send(Networking::Packet("Hello from server"), addr); })
            .transform([](auto &&conn)
                       { std::cout << "Packet sent" << std::endl;
                        return conn.get().close(); });
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