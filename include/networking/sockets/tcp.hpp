#pragma once

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