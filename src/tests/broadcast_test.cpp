#include "tests/broadcast_test.hpp"
#include "networking/sockets/udp.hpp"
#include "common/optional.hpp"

namespace Sockets = Networking::Sockets;
namespace Addr = Networking::Addresses;

opt::optional<Addr::Address> broad_print_packet(const std::pair<Networking::Packet, Addr::Address> &packet)
{
    std::cout << "Packet received: " << packet.first << std::endl;
    std::cout << "From: " << packet.second << std::endl;
    return packet.second;
}

int broad_client()
{
    try
    {
        Addr::Port port(8081);
        Sockets::UDP conn = Sockets::UDP(8080);
        std::cout << "Sending UDP broadcast at port " << port << std::endl;
        conn.send_broadcast(Networking::Packet("Hello from client"), port);
        std::cout << "Packet sent, waiting for a response..." << std::endl;
        conn.wait_and_receive_packet()
            .and_then(broad_print_packet);
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
        conn.wait_and_receive_packet()
            .and_then(broad_print_packet)
            .and_then([&conn](const Addr::Address &addr) -> opt::optional<Addr::Address>
                      { conn.send(Networking::Packet("Hello from server"), addr); 
                        std::cout << "Packet sent" << std::endl;
                        return addr; });
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