#include "networking/sockets/udp.hpp"

namespace Networking::Sockets
{
    UDP::UDP(const Networking::Addresses::Port &port) : UDP()
    {
        Networking::Addresses::Address address;
        address.setPort(port);
        if (!bind(address).has_value())
        {
            throw socket_error("Error binding");
        }
    }

    UDP::~UDP()
    {
        // close();
    }

    opt::optional<std::reference_wrapper<UDP>> UDP::send(const payload_t &message, const Networking::Addresses::Address &addr)
    {
        if (!checkOpen())
            return std::nullopt;
        const auto &address = addr.getAddr();
        const auto bytes_sent = ::sendto(getSocket(), reinterpret_cast<const char *>(message.data()), message.size(), 0, (sockaddr *)&address, sizeof(address));
        if (bytes_sent == SOCK_ERROR)
        {
            return std::nullopt;
        }
        return *this;
    }

    opt::optional<std::reference_wrapper<UDP>> UDP::send(const Networking::Packet &packet, const Networking::Addresses::Address &addr)
    {
        return send(packet.serialize(), addr);
    }

    opt::optional<std::pair<payload_t, Networking::Addresses::Address>> UDP::receive()
    {
        if (!checkOpen() || !getBound())
            return std::nullopt;

        Networking::Addresses::addr_t recieved_addr;
        auto addr_len = sizeof(recieved_addr);

        payload_t buffer(BUFFER_SIZE, 0);

        auto bytes_received = ::recvfrom(getSocket(), reinterpret_cast<char *>(buffer.data()), buffer.size(), 0, (sockaddr *)&recieved_addr, (socklen_t *)&addr_len);
        // If we didn't receive any bytes it may return 0 or -1
        if (bytes_received == 0 || bytes_received == SOCK_ERROR || !Networking::checkMagicNumber(buffer))
        {
            return std::nullopt;
        }

        return std::make_pair(buffer, Networking::Addresses::Address(recieved_addr));
    }

    opt::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> UDP::receive_packet()
    {
        return receive().transform([](const auto &received)
                                   { return std::make_pair(Networking::Packet(received.first), received.second); });
    }

    opt::optional<std::pair<payload_t, Networking::Addresses::Address>> UDP::wait_and_receive(std::chrono::milliseconds timeout)
    {
        if (!checkOpen() || !getBound())
            return std::nullopt;

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(getSocket(), &read_fds);

        timeval tv;
        tv.tv_sec = timeout.count() / 1000;
        tv.tv_usec = (timeout.count() % 1000) * 1000;

        auto select_result = ::select(getSocket() + 1, &read_fds, nullptr, nullptr, timeout != std::chrono::milliseconds::zero() ? &tv : nullptr);
        if (select_result == SOCK_ERROR || select_result == 0)
        {
            return std::nullopt;
        }

        return receive();
    }

    opt::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> UDP::wait_and_receive_packet(std::chrono::milliseconds timeout)
    {
        return wait_and_receive(timeout)
            .transform([](const auto &received)
                       { return std::make_pair(Networking::Packet(received.first), received.second); });
    }

    opt::optional<std::pair<payload_t, Networking::Addresses::Address>> UDP::wait_and_receive()
    {
        return wait_and_receive(std::chrono::milliseconds(0));
    }

    opt::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> UDP::wait_and_receive_packet()
    {
        return wait_and_receive_packet(std::chrono::milliseconds(0));
    }

    opt::optional<std::reference_wrapper<UDP>> UDP::send_broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port)
    {
        Networking::Addresses::Address broadcast_address(Networking::Addresses::BROADCAST_IP, port);
        return this->setOpt(SOL_SOCKET, SO_BROADCAST, 1)
            .transform([&](auto &&)
                       { return this->send(packet, broadcast_address); })
            .transform([&](auto &&)
                       { return this->setOpt(SOL_SOCKET, SO_BROADCAST, 0); })
            .transform([&](auto &&)
                       { return std::ref(*this); });
    }

    opt::optional<std::reference_wrapper<UDP>> UDP::send_wakeup(const MacAddress &mac)
    {
        constexpr auto MAGIC_PORT = 9;
        const auto magic_packet = Networking::Packet::createMagicPacket(mac);
        return send_broadcast(magic_packet, Networking::Addresses::Port(MAGIC_PORT));
    }

    success_t UDP::broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port)
    {
        Sockets::UDP udp;
        return udp.send_broadcast(packet, port)
            .transform([&udp](auto &&)
                       { return udp.close(); })
            .value_or(false);
    }

    success_t UDP::broadcast_wakeup(const MacAddress &mac)
    {
        Sockets::UDP udp;
        return udp.send_wakeup(mac)
            .transform([&udp](auto &&)
                       { return udp.close(); })
            .value_or(false);
    }
}