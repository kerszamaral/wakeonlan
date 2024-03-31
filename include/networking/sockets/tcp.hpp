#pragma once

#include "networking/packet.hpp"

#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class TCP : public Socket
    {
    protected:
        TCP() : Socket(Type::TCP) {}

    public:
        TCP(socket_t s) : Socket(s) {}
        TCP(const Networking::Addresses::Address &addr);
        TCP(const std::string &address) : TCP(Networking::Addresses::Address(address)) {}
        ~TCP();

        void send(const std::string &message) const;
        void send(const Networking::Packet &packet) const;
        std::string receive() const;
        Networking::Packet receive_packet() const;
    };

    class TCPServer : private TCP
    {
    private:
        constexpr static int MAX_CONNECTIONS = SOMAXCONN;
        Networking::Addresses::Address addr;

    public:
        TCPServer(const Networking::Addresses::Port &server_port);
        TCPServer(uint16_t server_port) : TCPServer(Networking::Addresses::Port(server_port)) {}

        // Wait for a connection on a server, returns a new TCP socket
        TCP wait_for_connection();
    };
}