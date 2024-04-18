#pragma once

#include <cstdint>

#include "networking/packets/util.hpp"
#include "common/format.hpp"

namespace Networking::Packets
{
    class Header
    {
    private:
        magic_number_t magic_number = MAGIC_NUMBER;
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
            to_bytes(data, this->magic_number);
            to_bytes(data, fmt::to_underlying(this->type));
            to_bytes(data, this->seqn);
            to_bytes(data, this->length);
            to_bytes(data, this->timestamp);
            return data;
        }
        constexpr payload_t serialize() const noexcept
        {
            payload_t data;
            data.reserve(this->size());
            return serialize(data);
        }

        constexpr payload_t::const_iterator deserialize(const payload_t &data) noexcept
        {
            constexpr auto packet_type_size = sizeof(PacketType) / sizeof(uint8_t);
            constexpr auto uint16_size = sizeof(uint16_t) / sizeof(uint8_t);
            constexpr auto magic_number_size = sizeof(magic_number_t) / sizeof(uint8_t);

            const auto &typeBegin = data.begin() + magic_number_size; // Skip magic number
            this->type = from_bytes<PacketType>(typeBegin);

            const auto &seqnBegin = typeBegin + packet_type_size;
            this->seqn = from_bytes<uint16_t>(seqnBegin);

            const auto &lengthBegin = seqnBegin + uint16_size;
            this->length = from_bytes<uint16_t>(lengthBegin);

            const auto &timestampBegin = lengthBegin + uint16_size;
            this->timestamp = from_bytes<uint16_t>(timestampBegin);

            return timestampBegin + uint16_size;
        }

        constexpr static size_t size() noexcept { return sizeof(Header); }

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