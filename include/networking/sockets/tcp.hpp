#pragma once

#include <tuple>
#include <optional>
#include <functional>

#include "networking/packet.hpp"

#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class TCP : protected Socket
    {
    private:
        Networking::Addresses::Address addr;

    protected:
        TCP() : Socket(Type::TCP) {}
        std::optional<std::reference_wrapper<TCP>> send(const payload_t &message);
        std::optional<payload_t> receive();
        // TCP(Socket s) : Socket(s) {}

    public:
        TCP(Socket s, const Networking::Addresses::Address &address) : Socket(s), addr(address) {}
        TCP(const std::pair<Socket, Networking::Addresses::Address> &pair) : TCP(pair.first, pair.second) {}
        TCP(const Networking::Addresses::Address &addr);
        TCP(const std::string &address) : TCP(Networking::Addresses::Address(address)) {}
        ~TCP();

        std::optional<std::reference_wrapper<TCP>> send(const Networking::Packet &packet);
        std::optional<Networking::Packet> receive_packet();
        Networking::Addresses::Address getAddress() const { return addr; }

        success_t close() { return Socket::close(); }
    };

    class TCPServer : private TCP
    {
    private:
        constexpr static int MAX_CONNECTIONS = SOMAXCONN;
        Networking::Addresses::Port port;

    public:
        TCPServer(const Networking::Addresses::Port &server_port);
        TCPServer(uint16_t server_port) : TCPServer(Networking::Addresses::Port(server_port)) {}

        // Wait for a connection on a server, returns a new TCP socket
        std::optional<TCP> wait_for_connection();
        Networking::Addresses::Port getPort() const { return port; }
        success_t close() { return Socket::close(); }
    };
}