#pragma once

#include <tuple>

#include "networking/packet.hpp"

#include "networking/sockets/socket.hpp"

namespace Networking::Sockets
{
    class TCP : public Socket
    {
    private:
        Networking::Addresses::Address addr;

    protected:
        TCP() : Socket(Type::TCP) {}
        void send(const payload_t &message) const;
        payload_t receive() const;
        // TCP(Socket s) : Socket(s) {}

    public:
        TCP(Socket s, const Networking::Addresses::Address &address) : Socket(s), addr(address) {}
        TCP(const Networking::Addresses::Address &addr);
        TCP(const std::string &address) : TCP(Networking::Addresses::Address(address)) {}
        ~TCP();

        void send(const Networking::Packet &packet) const;
        Networking::Packet receive_packet() const;
        Networking::Addresses::Address getAddress() const { return addr; }
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
        TCP wait_for_connection();
        Networking::Addresses::Port getPort() const { return port; }
    };
}