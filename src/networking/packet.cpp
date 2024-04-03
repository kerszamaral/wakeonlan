#include "networking/packet.hpp"

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

    payload_t &Body::serialize(payload_t &data) const
    {
        data.insert(data.end(), this->payload.begin(), this->payload.end());
        return data;
    }

    payload_t Body::serialize() const
    {
        return this->payload;
    }

    payload_t::const_iterator Body::deserialize(const payload_t &data)
    {
        this->payload = data;
        return data.end();
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
        body.deserialize(payload_t(it, data.end()));
        return *this;
    }
}