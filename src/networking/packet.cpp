#include "networking/packet.hpp"

#include <iomanip>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

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
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->type), packet_type_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->seqn), uint16_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->length), uint16_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->timestamp), uint16_size);
        return data;
    }

    payload_t Header::serialize() const
    {
        constexpr auto packet_type_size = sizeof(PacketType);
        constexpr auto uint16_size = sizeof(uint16_t);
        payload_t data(packet_type_size + uint16_size * 3);
        serialize(data);
        return data;
    }

    payload_t::const_iterator Header::deserialize(const payload_t &data)
    {
        constexpr auto packet_type_size = sizeof(PacketType);
        constexpr auto uint16_size = sizeof(uint16_t);
        auto it = data.begin();
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
            this->payload = payload_t(data.begin(), data.end());
            return data.end();
        case PacketType::STR:
            auto it = data.begin();
            this->payload = std::string(it, data.end());
            return data.end();
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
        payload_t data(header.size() + body.size());
        header.serialize(data);
        return body.serialize(data);
    }

    Packet &Packet::deserialize(const payload_t &data)
    {
        auto it = header.deserialize(data);
        body.deserialize(payload_t(it, data.end()), header.getType());
        return *this;
    }
}