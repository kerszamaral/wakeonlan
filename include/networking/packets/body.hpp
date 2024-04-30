#pragma once

#include "networking/packets/util.hpp"

namespace Networking::Packets
{
    template <class>
    inline constexpr bool always_false_v = false;

    class Body
    {
    private:
        body_t payload;

    public:
        Body() : payload(payload_t()) {}
        Body(const payload_t &payload) : payload(payload) {}
        Body(const std::string &payload) : payload(payload) {}
        Body(const body_t &payload) : payload(payload) {}
        Body(const payload_t::const_iterator &begin, const payload_t::const_iterator &end) : payload(payload_t(begin, end)) {}
        Body(const std::string::const_iterator &begin, const std::string::const_iterator &end) : payload(std::string(begin, end)) {}

        const body_t &getPayload() const noexcept { return payload; }
        body_t &getPayload() noexcept { return payload; }
        size_t size() const noexcept
        {
            size_t size = 0;
            auto handle_cases = [&size](auto &&arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>)
                {
                    size = arg.size();
                }
                else if constexpr (std::is_same_v<T, payload_t>)
                {
                    size = arg.size();
                }
                else if constexpr (std::is_same_v<T, SSE_Data>)
                {
                    size = arg.first.size() + arg.second.size();
                }
                else if constexpr (std::is_same_v<T, Addresses::Mac>)
                {
                    constexpr auto FFSize = 6;
                    constexpr auto MACSize = 6;
                    constexpr auto MACRepeat = 16;
                    size = FFSize + MACSize * MACRepeat;
                }
                else
                {
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            };
            std::visit(handle_cases, this->payload);
            return size;
        }

        payload_t &serialize(payload_t &data) const noexcept
        {
            auto handle_cases = [&data](auto &&arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>)
                {
                    to_bytes(data, arg.begin(), arg.size());
                }
                else if constexpr (std::is_same_v<T, payload_t>)
                {
                    data.insert(data.end(), arg.begin(), arg.end());
                }
                else if constexpr (std::is_same_v<T, SSE_Data>)
                {
                    const auto [hostname, mac] = arg;
                    to_bytes(data, mac.data().begin(), mac.data().size());
                    to_bytes(data, hostname.begin(), hostname.size());
                }
                else if constexpr (std::is_same_v<T, Addresses::Mac>)
                {
                    constexpr auto FFSize = 6;
                    constexpr uint8_t FFbyte = 0xFF;
                    constexpr auto MACRepeat = 16;
                    for (int i = 0; i < FFSize; i++)
                    {
                        data.push_back(FFbyte);
                    }
                    const auto macData = arg.data();
                    for (int i = 0; i < MACRepeat; i++)
                    {
                        to_bytes(data, macData.begin(), macData.size());
                    }
                }
                else
                {
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            };
            std::visit(handle_cases, this->payload);
            return data;
        }

        payload_t serialize() const noexcept
        {
            payload_t data;
            data.reserve(this->size());
            return serialize(data);
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch" // Makes switch exhaustive
        payload_t::const_iterator deserialize(const payload_t &data, const PacketType &type, const uint16_t size)
        {
            //! Switch statement is exhaustive and needs to be in order.
            switch (type)
            {
            case PacketType::DATA:
            case PacketType::SSR:
            case PacketType::SSR_ACK:
            {
                // Copy the data from the iterator to the end of the data
                const auto &begin = data.begin();
                const auto &end = begin + size;
                this->payload = payload_t(begin, end);
                return end;
            }
            case PacketType::STR:
            case PacketType::SSE:
            {
                const auto &begin = data.begin();
                const auto &end = begin + size;
                this->payload = std::string(begin, end);
                return end;
            }
            case PacketType::SSD:
            case PacketType::SSD_ACK:
            {
                const auto &begin = data.begin();
                const auto &macEnd = begin + Addresses::MAC_ADDR_LEN;
                const auto &hostnameEnd = begin + size;
                if (macEnd > hostnameEnd)
                {
                    this->payload = std::make_pair(std::string(), Addresses::Mac());
                    return data.end();
                }

                Addresses::mac_addr_t mac;
                std::copy(begin, macEnd, mac.begin());

                const auto &hostname = std::string(macEnd, hostnameEnd);

                this->payload = std::make_pair(hostname, Addresses::Mac(mac));
                return hostnameEnd;
            }
            case PacketType::MAGIC:
            {
                constexpr auto FFSize = 6;
                constexpr auto MACSize = Addresses::MAC_ADDR_LEN;
                const auto &macBegin = data.begin() + FFSize; // Skip FF bytes
                const auto &macEnd = macBegin + MACSize;
                Addresses::mac_addr_t mac;
                std::copy(macBegin, macEnd, mac.begin()); // Copy the first MAC address
                this->payload = Addresses::Mac(mac);
                return data.end();
            }
            }
            return data.end(); // unreachable
        }
#pragma GCC diagnostic pop

        std::string to_string() const
        {
            if (std::holds_alternative<std::string>(this->payload))
                return std::get<std::string>(this->payload);
            else if (std::holds_alternative<payload_t>(this->payload))
                return std::string(std::get<payload_t>(this->payload).begin(), std::get<payload_t>(this->payload).end());

            return "";
        }

        friend std::ostream &operator<<(std::ostream &os, const Body &body)
        {
            os << body.to_string();
            return os;
        }
    };
}