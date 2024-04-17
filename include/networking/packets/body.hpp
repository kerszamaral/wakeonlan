#pragma once

#include "networking/packets/payload.hpp"

namespace Networking::Packets
{
    template <class>
    inline constexpr bool always_false_v = false;

    class Body
    {
    private:
        body_t payload;

    public:
        constexpr Body() : payload(payload_t()) {}
        constexpr Body(const payload_t &payload) : payload(payload) {}
        constexpr Body(const std::string &payload) : payload(payload) {}
        constexpr Body(const body_t &payload) : payload(payload) {}
        constexpr Body(const payload_t::const_iterator &begin, const payload_t::const_iterator &end) : payload(payload_t(begin, end)) {}
        constexpr Body(const std::string::const_iterator &begin, const std::string::const_iterator &end) : payload(std::string(begin, end)) {}

        constexpr const body_t &getPayload() const noexcept { return payload; }
        constexpr body_t &getPayload() noexcept { return payload; }
        constexpr size_t size() const noexcept
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

        constexpr payload_t &serialize(payload_t &data) const noexcept
        {
            auto handle_cases = [&data](auto &&arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>)
                {
                    extendBytes(data, reinterpret_cast<const uint8_t *>(arg.c_str()), arg.size());
                }
                else if constexpr (std::is_same_v<T, payload_t>)
                {
                    data.insert(data.end(), arg.begin(), arg.end());
                }
                else if constexpr (std::is_same_v<T, SSE_Data>)
                {
                    const auto [hostname, mac] = arg;
                    extendBytes(data, mac.data().data(), mac.data().size());
                    extendBytes(data, reinterpret_cast<const uint8_t *>(hostname.c_str()), hostname.size());
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
                        extendBytes(data, macData.data(), macData.size());
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

        constexpr payload_t serialize() const noexcept
        {
            payload_t data;
            data.reserve(this->size());
            return serialize(data);
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch" // Makes switch exhaustive
        constexpr payload_t::const_iterator deserialize(const payload_t &data, const PacketType &type)
        {
            //! Switch statement is exhaustive and needs to be in order.
            switch (type)
            {
            case PacketType::DATA:
            {
                this->payload = payload_t(data.begin(), data.end());
                return data.end();
            }
            case PacketType::SSE:
            case PacketType::STR:
            {
                auto it = data.begin();
                std::string str(it, data.end());
                str.erase(str.find_first_of('\0'));
                this->payload = str;
                return data.end();
            }
            case PacketType::SSD:
            case PacketType::SSD_ACK:
            {
                auto it = data.begin();
                Addresses::mac_addr_t mac;
                std::copy(it, it + mac.size(), mac.begin());
                it += mac.size();
                std::string hostname(it, data.end());
                hostname.erase(hostname.find_first_of('\0'));
                this->payload = std::make_pair(hostname, Addresses::Mac(mac));
                return data.end();
            }
            case PacketType::MAGIC:
            {
                constexpr auto FFSize = 6;
                constexpr auto MACSize = Addresses::MAC_ADDR_LEN;
                auto it = data.begin();
                it += FFSize; // Skip FF bytes
                Addresses::mac_addr_t mac;
                std::copy(it, it + MACSize, mac.begin()); // Copy the first MAC address
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