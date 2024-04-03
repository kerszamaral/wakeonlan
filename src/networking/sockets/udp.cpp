#include "networking/sockets/udp.hpp"

namespace Networking::Sockets
{
    UDP::UDP(const Networking::Addresses::Port &port) : UDP()
    {
        Networking::Addresses::Address address;
        address.setPort(port);
        bind(address);
    }

    UDP::~UDP()
    {
        // close();
    }

    void UDP::send(const payload_t &message, const Networking::Addresses::Address &addr) const
    {
        checkOpen();
        const auto &address = addr.getAddr();
        const auto bytes_sent = ::sendto(getSocket(), reinterpret_cast<const char *>(message.data()), message.size(), 0, (sockaddr *)&address, sizeof(address));
        if (bytes_sent == SOCK_ERROR)
        {
            throw_error("sendto failed");
        }
    }

    void UDP::send(const Networking::Packet &packet, const Networking::Addresses::Address &addr) const
    {
        send(packet.serialize(), addr);
    }

    std::optional<std::pair<payload_t, Networking::Addresses::Address>> UDP::receive() const
    {
        checkOpen();
        if (!getBound())
        {
            throw_error("Socket not bound");
        }

        Networking::Addresses::addr_t recieved_addr;
        auto addr_len = sizeof(recieved_addr);

        payload_t buffer(BUFFER_SIZE, 0);

        auto bytes_received = ::recvfrom(getSocket(), reinterpret_cast<char *>(buffer.data()), buffer.size(), 0, (sockaddr *)&recieved_addr, (socklen_t *)&addr_len);
        // If we didn't receive any bytes it may return 0 or -1
        if (bytes_received == 0 || bytes_received == ERROR || !Networking::checkMagicNumber(buffer))
        {
            return std::nullopt;
        }

        return std::make_pair(buffer, Networking::Addresses::Address(recieved_addr));
    }

    std::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> UDP::receive_packet() const
    {
        auto received = receive();
        if (!received.has_value())
        {
            return std::nullopt;
        }

        Networking::Packet packet;
        packet.deserialize(received.value().first);
        return std::make_pair(packet, received.value().second);
    }

    std::optional<std::pair<payload_t, Networking::Addresses::Address>> UDP::wait_and_receive(uint32_t timeout) const
    {
        checkOpen();
        if (!getBound())
        {
            throw_error("Socket not bound");
        }

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(getSocket(), &read_fds);

        timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        auto select_result = ::select(getSocket() + 1, &read_fds, nullptr, nullptr, timeout ? &tv : nullptr);
        if (select_result == SOCK_ERROR)
        {
            throw_error("select failed");
        }

        if (select_result == 0)
        {
            return std::nullopt;
        }

        return receive();
    }

    std::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> UDP::wait_and_receive_packet(uint32_t timeout) const
    {
        auto received = wait_and_receive(timeout);
        if (!received.has_value())
        {
            return std::nullopt;
        }

        Networking::Packet packet;
        packet.deserialize(received.value().first);
        return std::make_pair(packet, received.value().second);
    }

    std::pair<payload_t, Networking::Addresses::Address> UDP::wait_and_receive() const
    {
        auto received = wait_and_receive(0);
        if (!received.has_value())
        {
            throw_error("No message received");
        }
        return received.value();
    }

    std::pair<Networking::Packet, Networking::Addresses::Address> UDP::wait_and_receive_packet() const
    {
        auto received = wait_and_receive_packet(0);
        if (!received.has_value())
        {
            throw_error("No message received");
        }
        return received.value();
    }

    void UDP::send_broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port)
    {
        this->setOpt(SOL_SOCKET, SO_BROADCAST, 1);
        Networking::Addresses::Address broadcast_address(Networking::Addresses::BROADCAST_IP, port);
        send(packet, broadcast_address);
        this->setOpt(SOL_SOCKET, SO_BROADCAST, 0);
    }

    void UDP::broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port)
    {
        Sockets::UDP udp;
        udp.send_broadcast(packet, port);
        udp.close();
    }
}