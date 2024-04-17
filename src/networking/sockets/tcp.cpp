#include "networking/sockets/tcp.hpp"

namespace Networking::Sockets
{
    TCP::TCP(const Networking::Addresses::Address &addr) : TCP()
    {
        this->addr = addr;
        if (!this->connect(addr))
        {
            throw socket_error("Failed to connect to " + addr.to_string());
        }
    }

    TCP::~TCP()
    {
        // close();
    }

    opt::optional<std::reference_wrapper<TCP>> TCP::send(const Packets::payload_t &message)
    {
        if (!checkOpen())
            return std::nullopt;
        auto bytes_sent = ::send(getSocket(), reinterpret_cast<const char *>(message.data()), message.size(), 0);
        if (bytes_sent == SOCK_ERROR)
        {
            return std::nullopt;
        }
        return *this;
    }

    opt::optional<std::reference_wrapper<TCP>> TCP::send(const Networking::Packets::Packet &packet)
    {
        return send(packet.serialize());
    }

    opt::optional<Packets::payload_t> TCP::receive()
    {
        if (!checkOpen())
            return std::nullopt;
        Packets::payload_t buffer;
        do
        {
            buffer.resize(BUFFER_SIZE, 0);
            auto bytes_received = ::recv(getSocket(), reinterpret_cast<char *>(buffer.data()), buffer.size(), 0);
            if (bytes_received == SOCK_ERROR)
            {
                return std::nullopt;
            }
            buffer.resize(bytes_received);
        } while (!Networking::Packets::checkMagicNumber(buffer));

        return buffer;
    }

    opt::optional<Networking::Packets::Packet> TCP::receive_packet()
    {
        return receive().transform([](const auto &payload)
                                   { return Networking::Packets::Packet(payload); });
    }

    TCPServer::TCPServer(const Networking::Addresses::Port &server_port) : TCP()
    {
        this->port = server_port;
        Networking::Addresses::Address addr;
        addr.setPort(server_port);

        constexpr int opt = 1;
        if (!this->setOpt(SOL_SOCKET, SO_REUSEADDR, opt))
        {
            throw socket_error("Failed to set SO_REUSEADDR");
        }

        if (!this->bind(addr))
        {
            throw socket_error("Failed to bind to " + addr.to_string());
        }

        if (!this->listen(MAX_CONNECTIONS))
        {
            throw socket_error("Failed to listen on " + addr.to_string());
        }
    }

    opt::optional<TCP> TCPServer::wait_for_connection()
    {
        return accept().transform([](const auto &client)
                                  { return TCP(client); });
    }
}