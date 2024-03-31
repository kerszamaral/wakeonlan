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

        void send(const std::string &message) const;
        std::string receive() const;

        // Operators
        friend std::ostream &operator<<(std::ostream &os, const TCP &s)
        {
            return os << s.receive();
        }

        friend std::istream &operator>>(std::istream &is, const TCP &s)
        {
            std::string message;
            is >> message;
            s.send(message);
            return is;
        }
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