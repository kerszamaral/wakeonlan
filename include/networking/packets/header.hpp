#pragma once

#include <cstdint>

#include "networking/packets/payload.hpp"
#include "common/format.hpp"

namespace Networking::Packets
{
    constexpr uint16_t MAGIC_NUMBER = 0xCA31;

    constexpr inline bool checkMagicNumber(const payload_t &data)
    {
        return *reinterpret_cast<const uint16_t *>(data.data()) == MAGIC_NUMBER;
    }

    class Header
    {
    private:
        uint16_t magic_number = MAGIC_NUMBER;
        PacketType type;
        uint16_t seqn;
        uint16_t length;
        uint16_t timestamp;

    public:
        constexpr Header() noexcept : type(PacketType::DATA), seqn(0), length(0), timestamp(0) {}
        constexpr Header(PacketType type, uint16_t seqn, uint16_t length, uint16_t timestamp) noexcept : type(type), seqn(seqn), length(length), timestamp(timestamp) {}
        constexpr PacketType getType() const noexcept { return type; }
        constexpr uint16_t getSeqn() const noexcept { return seqn; }
        constexpr uint16_t getLength() const noexcept { return length; }
        constexpr uint16_t getTimestamp() const noexcept { return timestamp; }
        constexpr payload_t &serialize(payload_t &data) const noexcept
        {
            extendBytes(data, this->magic_number);
            extendBytes(data, fmt::to_underlying(this->type));
            extendBytes(data, this->seqn);
            extendBytes(data, this->length);
            extendBytes(data, this->timestamp);
            return data;
        }
        payload_t serialize() const noexcept
        {
            payload_t data;
            data.reserve(this->size());
            return serialize(data);
        }
        payload_t::const_iterator deserialize(const payload_t &data) noexcept
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

        constexpr size_t size() const noexcept { return sizeof(Header); }

        std::string to_string() const
        {
            return "Type: " + std::to_string(static_cast<uint16_t>(type)) + ", Seqn: " + std::to_string(seqn) + ", Length: " + std::to_string(length) + ", Timestamp: " + std::to_string(timestamp);
        }

        friend std::ostream &operator<<(std::ostream &os, const Header &header)
        {
            os << header.to_string();
            return os;
        }
    };
}