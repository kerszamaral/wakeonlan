#include "networking/sockets/tcp.hpp"

namespace Networking::Sockets
{
    TCP::TCP(const Networking::Addresses::Address &addr) : TCP()
    {
        connect(addr.getAddr());
    }

    TCPServer::TCPServer(const Networking::Addresses::Port &server_port) : TCP()
    {
        addr.setPort(server_port);

        const int opt = 1;
        this->setOpt(SOL_SOCKET, SO_REUSEADDR, opt);

        this->bind(addr);

        constexpr auto backlog = SOMAXCONN;
        this->listen(backlog);
    }

    TCP::~TCP()
    {
        close();
    }

    TCP TCPServer::await_connection()
    {
        return TCP(this->accept(addr));
    }
}