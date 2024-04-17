#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <variant>
#include <tuple>

#include "common/pcinfo.hpp"

namespace Networking
{
    enum class PacketType : uint16_t
    {
        DATA = 0x0000,
        STR = 0x0001,
        SSD = 0x0002,     // Sleep Service Discovery
        SSD_ACK = 0x0003, // Sleep Service Discovery Acknowledgement
        SSE = 0x0004,     // Sleep Service Exit
        MAGIC = 0xFFFF,   // Magic Packet
        // DATA = 0x0001,
        // CMD = 0x0002,
        // ACK = 0x0003,
        // NACK = 0x0004,
    };

    typedef std::vector<uint8_t> payload_t;

    typedef std::pair<PC::hostname_t, Addresses::Mac> SSE_Data;

    typedef std::variant<std::string, payload_t, SSE_Data, Addresses::Mac> body_t;

    constexpr uint16_t MAGIC_NUMBER = 0xCA31;

    constexpr inline bool checkMagicNumber(const payload_t &data)
    {
        return *reinterpret_cast<const uint16_t *>(data.data()) == MAGIC_NUMBER;
    }

    constexpr inline void extendBytes(payload_t &vec, const uint8_t *data, size_t size)
    {
        vec.insert(vec.end(), data, data + size);
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
        payload_t &serialize(payload_t &data) const noexcept
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

        const body_t &getPayload() const { return payload; }
        body_t &getPayload() { return payload; }
        size_t size() const;

        payload_t &serialize(payload_t &data) const;
        payload_t serialize() const;
        payload_t::const_iterator deserialize(const payload_t &data, const PacketType &type);

        std::string to_string() const;

        friend std::ostream &operator<<(std::ostream &os, const Body &body)
        {
            os << body.to_string();
            return os;
        }
    };

    class Packet
    {
    private:
        Header header;
        Body body;

    public:
        Packet() : header(), body() {}
        Packet(const payload_t &data);
        Packet(const body_t &payload);
        Packet(PacketType type);
        Packet(PacketType type, const body_t &payload);
        Packet(const Header &header, const Body &body) : header(header), body(body) {}
        Packet(PacketType type, uint16_t seqn, uint16_t timestamp, const payload_t &payload) : header(type, seqn, payload.size(), timestamp), body(payload) {}
        Packet(PacketType type, uint16_t seqn, uint16_t timestamp, const std::string &payload) : header(type, seqn, payload.length(), timestamp), body(payload) {}
        payload_t serialize() const;
        Packet &deserialize(const payload_t &data);

        PacketType getType() const { return header.getType(); }
        Header &getHeader() { return header; }
        const Header &getHeader() const { return header; }
        Body &getBody() { return body; }
        const Body &getBody() const { return body; }
        size_t size() const { return header.size() + body.size(); }

        Packet &setBody(const body_t &payload);

        static Packet createPacket(const body_t &payload);
        static Packet createMagicPacket(const Addresses::Mac &mac);

        std::string to_string() const
        {
            return header.to_string() + "\n" + body.to_string();
        }

        friend std::ostream &operator<<(std::ostream &os, const Packet &packet)
        {
            os << packet.to_string();
            return os;
        }
    };
}