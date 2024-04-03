#include "networking/packet.hpp"

namespace Networking
{
    inline void extendBytes(payload_t &vec, const uint8_t *data, size_t size)
    {
        vec.insert(vec.end(), data, data + size);
    }

    payload_t Packet::serialize() const
    {
        constexpr auto packet_type_size = sizeof(PacketType);
        constexpr auto uint16_size = sizeof(uint16_t);
        constexpr auto header_size = packet_type_size + uint16_size * 3; // type, seqn, length, timestamp
        payload_t data(header_size + this->length);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->type), packet_type_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->seqn), uint16_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->length), uint16_size);
        extendBytes(data, reinterpret_cast<const uint8_t *>(&this->timestamp), uint16_size);
        data.insert(data.end(), this->payload.begin(), this->payload.end());
        return data;
    }

    void Packet::deserialize(const payload_t &data)
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
        this->payload = payload_t(it, data.end());
    }
}