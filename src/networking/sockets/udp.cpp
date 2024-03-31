#include "networking/sockets/udp.hpp"

namespace Networking::Sockets
{
    UDP::~UDP()
    {
        close();
    }

    void UDP::send(const std::string &message, const Networking::Addresses::Address &addr) const
    {
        checkOpen();
        const auto &address = addr.getAddr();
        const auto bytes_sent = ::sendto(getSocket(), message.c_str(), message.length(), 0, (sockaddr *)&address, sizeof(address));
        if (bytes_sent == ERROR)
        {
            throw_error("sendto failed");
        }
    }

    std::optional<std::string> UDP::receive(Networking::Addresses::Address &addr) const
    {
        checkOpen();
        const auto &address = addr.getAddr();
        std::string buffer(BUFFER_SIZE, 0);
        auto addr_len = sizeof(address);
        auto bytes_received = ::recvfrom(getSocket(), buffer.data(), buffer.length(), 0, (sockaddr *)&address, (socklen_t *)&addr_len);
        if (bytes_received == ERROR)
        {
            throw_error("recvfrom failed");
        }
        if (bytes_received == 0)
        {
            return std::nullopt;
        }
        return buffer;
    }
}