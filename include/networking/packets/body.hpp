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
                else if constexpr (std::is_same_v<T, SSREP_Data>)
                {
                    size = arg.second.size();
                }
                else if constexpr (std::is_same_v<T, uint32_t>)
                {
                    size = sizeof(uint32_t);
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
                else if constexpr (std::is_same_v<T, SSREP_Data>)
                {
                    to_bytes(data, arg.first);
                    for (const auto &[hostname, pc] : arg.second)
                    {
                        to_bytes(data, hostname.length());
                        to_bytes(data, hostname.begin(), hostname.end());
                        const auto &mac = pc.get_mac();
                        to_bytes(data, mac.data().begin(), mac.data().size());
                        const auto &ipv4 = pc.get_ipv4();
                        to_bytes(data, ipv4.to_network_order());
                        to_bytes(data, pc.get_status());
                        to_bytes(data, pc.get_is_manager());
                    }
                }
                else if constexpr (std::is_same_v<T, uint32_t>)
                {
                    to_bytes(data, arg);
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
            case PacketType::SSELFIN:
            case PacketType::SSELGT:
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
            case PacketType::SSREP:
            {
                PC::pc_map_t pc_map;
                auto num_entries = size;
                auto it = data.begin();
                const auto &version = from_bytes<std::uint32_t>(it);
                it += sizeof(std::uint32_t);

                while (num_entries)
                {
                    const auto &hostnameLen = from_bytes<std::size_t>(it);
                    it += sizeof(std::size_t);
                    const auto &hostnameEnd = it + hostnameLen;
                    const auto &hostname = std::string(it, hostnameEnd);
                    it = hostnameEnd;

                    Addresses::mac_addr_t mac;
                    const auto &macEnd = it + Addresses::MAC_ADDR_LEN;
                    std::copy(it, macEnd, mac.begin());
                    it = macEnd;

                    const auto &ipv4 = Addresses::IPv4(from_bytes<std::uint32_t>(it));
                    it += sizeof(std::uint32_t);
                
                    const auto &status = from_bytes<PC::STATUS>(it);
                    it += sizeof(PC::STATUS);

                    const auto &is_manager = from_bytes<bool>(it);
                    it += sizeof(bool);

                    const auto &pc = PC::PCInfo(hostname, Addresses::Mac(mac), ipv4, status, is_manager);
                    pc_map.insert_or_assign(hostname, pc);

                    num_entries--;
                }

                this->payload = std::make_pair(version, pc_map);
                return data.end();
            }
            case PacketType::SSR_ACK:
            case PacketType::SSEL:
            {
                const auto &value = from_bytes<std::uint32_t>(data.begin());
                this->payload = value;
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