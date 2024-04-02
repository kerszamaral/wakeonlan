#include "networking/sockets/tcp.hpp"

namespace Networking::Sockets
{
    TCP::TCP(const Networking::Addresses::Address &addr) : TCP()
    {
        connect(addr.getAddr());
    }

    TCP::~TCP()
    {
        // close();
    }

    void TCP::send(const std::string &message) const
    {
        checkOpen();
        auto bytes_sent = ::send(getSocket(), message.c_str(), message.length(), 0);
        if (bytes_sent == SOCK_ERROR)
        {
            throw_error("send failed");
        }
    }

    void TCP::send(const Networking::Packet &packet) const
    {
        send(packet.serialize());
    }

    std::string TCP::receive() const
    {
        checkOpen();
        std::string buffer(BUFFER_SIZE, 0);
        auto bytes_received = ::recv(getSocket(), buffer.data(), buffer.length(), 0);
        if (bytes_received == SOCK_ERROR)
        {
            throw_error("recv failed");
        }
        return buffer;
    }

    Networking::Packet TCP::receive_packet() const
    {
        Networking::Packet packet;
        packet.deserialize(receive());
        return packet;
    }

    TCPServer::TCPServer(const Networking::Addresses::Port &server_port) : TCP()
    {
        addr.setPort(server_port);

        constexpr int opt = 1;
        this->setOpt(SOL_SOCKET, SO_REUSEADDR, opt);

        this->bind(addr);

        this->listen(MAX_CONNECTIONS);
    }

    std::pair<TCP, Networking::Addresses::Address> TCPServer::wait_for_connection()
    {
        auto res = this->accept();
        return std::make_pair(TCP(res.first.getSocket()), res.second);
    }
}