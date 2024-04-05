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
        return receive()
            .and_then([](const auto &received) -> opt::optional<std::pair<Networking::Packet, Networking::Addresses::Address>>
                      { return std::make_pair(Networking::Packet(received.first), received.second); });
    }

    opt::optional<std::pair<payload_t, Networking::Addresses::Address>> UDP::wait_and_receive(uint32_t timeout)
    {
        if (!checkOpen() || !getBound())
            return std::nullopt;

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(getSocket(), &read_fds);

        timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        auto select_result = ::select(getSocket() + 1, &read_fds, nullptr, nullptr, timeout ? &tv : nullptr);
        if (select_result == SOCK_ERROR || select_result == 0)
        {
            return std::nullopt;
        }

        return receive();
    }

    opt::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> UDP::wait_and_receive_packet(uint32_t timeout)
    {
        return wait_and_receive(timeout)
            .and_then([](const auto &received) -> opt::optional<std::pair<Networking::Packet, Networking::Addresses::Address>>
                      { return std::make_pair(Networking::Packet(received.first), received.second); });
        // if (const auto &received = wait_and_receive(timeout))
        // {
        //     return std::make_pair(Networking::Packet(received->first), received->second);
        // }
        // return std::nullopt;
    }

    opt::optional<std::pair<payload_t, Networking::Addresses::Address>> UDP::wait_and_receive()
    {
        return wait_and_receive(0);
    }

    opt::optional<std::pair<Networking::Packet, Networking::Addresses::Address>> UDP::wait_and_receive_packet()
    {
        return wait_and_receive_packet(0);
    }

    opt::optional<std::reference_wrapper<UDP>> UDP::send_broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port)
    {
        Networking::Addresses::Address broadcast_address(Networking::Addresses::BROADCAST_IP, port);
        return setOpt(SOL_SOCKET, SO_BROADCAST, 1)
            .and_then([&](auto &&) -> opt::optional<std::reference_wrapper<UDP>>
                      { return this->send(packet, broadcast_address); })
            .and_then([&](auto &&) -> opt::optional<std::reference_wrapper<UDP>>
                      { this->setOpt(SOL_SOCKET, SO_BROADCAST, 0);
                          return *this; });
    }

    success_t UDP::broadcast(const Networking::Packet &packet, const Networking::Addresses::Port &port)
    {
        Sockets::UDP udp;
        return udp.send_broadcast(packet, port)
            .and_then([](auto udp) -> opt::optional<success_t>
                      { return udp.get().close(); })
            .value_or(false);
    }
}