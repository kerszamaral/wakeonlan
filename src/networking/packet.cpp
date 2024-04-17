#include "networking/packet.hpp"

#include <iomanip>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "common/pcinfo.hpp"

namespace Networking
{
    inline void extendBytes(payload_t &vec, const uint8_t *data, size_t size)
    {
        vec.insert(vec.end(), data, data + size);
    }

    payload_t &Header::serialize(payload_t &data) const
    {
        constexpr auto packet_type_size = sizeof(PacketType);
        constexpr auto uint16_size = sizeof(uint16_t);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->magic_number), uint16_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->type), packet_type_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->seqn), uint16_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->length), uint16_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->timestamp), uint16_size);
        return data;
    }

    payload_t Header::serialize() const
    {
        payload_t data;
        data.reserve(this->size());
        return serialize(data);
    }

    payload_t::const_iterator Header::deserialize(const payload_t &data)
    {
        constexpr auto packet_type_size = sizeof(PacketType);
        constexpr auto uint16_size = sizeof(uint16_t);
        auto it = data.begin();
        it += uint16_size; // Skip magic number
        this->type = *reinterpret_cast<const PacketType *>(&*it);
        it += packet_type_size;
        this->seqn = *reinterpret_cast<const uint16_t *>(&*it);
        it += uint16_size;
        this->length = *reinterpret_cast<const uint16_t *>(&*it);
        it += uint16_size;
        this->timestamp = *reinterpret_cast<const uint16_t *>(&*it);
        it += uint16_size;
        return it;
    }

    template <class>
    inline constexpr bool always_false_v = false;

    size_t Body::size() const
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

    payload_t &Body::serialize(payload_t &data) const
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

    payload_t Body::serialize() const
    {
        payload_t data;
        data.reserve(this->size());
        return serialize(data);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch" // Makes switch exhaustive
    payload_t::const_iterator Body::deserialize(const payload_t &data, const PacketType &type)
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

    std::string Body::to_string() const
    {
        if (std::holds_alternative<std::string>(this->payload))
            return std::get<std::string>(this->payload);
        else if (std::holds_alternative<payload_t>(this->payload))
            return std::string(std::get<payload_t>(this->payload).begin(), std::get<payload_t>(this->payload).end());

        return "";
    }

    payload_t Packet::serialize() const
    {
        payload_t data;
        data.reserve(this->size());
        header.serialize(data);
        return body.serialize(data);
    }

    Packet &Packet::deserialize(const payload_t &data)
    {
        auto it = header.deserialize(data);
        body.deserialize(payload_t(it, data.end()), header.getType());
        return *this;
    }

    Packet &Packet::setBody(const body_t &payload)
    {
        this->body = Body(payload);
        auto &head = this->header;
        auto handle_cases = [&head](auto &&arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>)
            {
                head = Header(PacketType::STR, 0, arg.size(), 0);
            }
            else if constexpr (std::is_same_v<T, payload_t>)
            {
                head = Header(PacketType::DATA, 0, arg.size(), 0);
            }
            else if constexpr (std::is_same_v<T, SSE_Data>)
            {
                head = Header(PacketType::SSD, 0, arg.first.size() + arg.second.size(), 0);
            }
            else if constexpr (std::is_same_v<T, Addresses::Mac>)
            {
                head = Header(PacketType::MAGIC, 0, 0, 0);
            }
            else
            {
                static_assert(always_false_v<T>, "non-exhaustive visitor!");
            }
        };
        std::visit(handle_cases, body.getPayload());
        return *this;
    }

    Packet::Packet(const payload_t &data) : header(), body()
    {
        this->deserialize(data);
    }

    Packet::Packet(const body_t &payload) : header(), body()
    {
        this->setBody(payload);
    }

    Packet::Packet(PacketType type, const body_t &payload)
    {
        this->body = Body(payload);
        this->header = Header(type, 0, this->body.size(), 0);
    }

    Packet Packet::createPacket(const body_t &payload)
    {
        return Packet(payload);
    }

    Packet Packet::createMagicPacket(const Addresses::Mac &mac)
    {
        return Packet(mac);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch" // Makes switch exhaustive
    Packet::Packet(PacketType type) : header(type, 0, 0, 0), body()
    {
        switch (type)
        {
        case PacketType::DATA:
            this->body = Body(payload_t());
            break;
        case PacketType::STR:
            this->body = Body(std::string());
            break;
        case PacketType::SSD:
        case PacketType::SSD_ACK:
            this->body = Body(std::make_pair(PC::getHostname(), Addresses::Mac::FromMachine().value()));
            break;
        case PacketType::SSE:
            this->body = Body(PC::getHostname());
            break;
        case PacketType::MAGIC:
            this->body = Body(Addresses::Mac());
            break;
        }
    }
#pragma GCC diagnostic pop
}