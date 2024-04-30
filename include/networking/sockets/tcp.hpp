#pragma once

#include <tuple>
#include "common/optional.hpp"
#include <functional>

#include "networking/packets/packet.hpp"

#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class TCP : protected Socket
    {
    private:
        Networking::Addresses::Address addr;

    protected:
        TCP() : Socket(Type::TCP) {}
        opt::optional<std::reference_wrapper<TCP>> send(const Packets::payload_t &message);
        opt::optional<Packets::payload_t> receive();
        // TCP(Socket s) : Socket(s) {}

    public:
        TCP(Socket s, const Networking::Addresses::Address &address) : Socket(s), addr(address) {}
        TCP(const std::pair<Socket, Networking::Addresses::Address> &pair) : TCP(pair.first, pair.second) {}
        TCP(const Networking::Addresses::Address &addr);
        TCP(const std::string &address) : TCP(Networking::Addresses::Address(address)) {}
        ~TCP();

        opt::optional<std::reference_wrapper<TCP>> send(const Networking::Packets::Packet &packet);
        opt::optional<Networking::Packets::Packet> receive_packet();
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
        opt::optional<TCP> wait_for_connection();
        Networking::Addresses::Port getPort() const { return port; }
        success_t close() { return Socket::close(); }
    };
}