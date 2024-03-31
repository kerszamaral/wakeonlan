#include "networking/packet.hpp"

namespace Networking
{
    std::string Packet::serialize() const
    {
        std::stringstream ss;
        ss.write(reinterpret_cast<const char *>(&this->type), sizeof(PacketType));
        ss.write(reinterpret_cast<const char *>(&this->seqn), sizeof(uint16_t));
        ss.write(reinterpret_cast<const char *>(&this->length), sizeof(uint16_t));
        ss.write(reinterpret_cast<const char *>(&this->timestamp), sizeof(uint16_t));
        ss << this->payload;
        return ss.str();
    }

    void Packet::deserialize(const std::string &data)
    {
        this->type = *reinterpret_cast<const PacketType *>(data.c_str());
        this->seqn = *reinterpret_cast<const uint16_t *>(data.c_str() + sizeof(PacketType));
        this->length = *reinterpret_cast<const uint16_t *>(data.c_str() + sizeof(PacketType) + sizeof(uint16_t));
        this->timestamp = *reinterpret_cast<const uint16_t *>(data.c_str() + sizeof(PacketType) + sizeof(uint16_t) + sizeof(uint16_t));
        this->payload = std::string(data.c_str() + sizeof(PacketType) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t), this->length);
    }
}