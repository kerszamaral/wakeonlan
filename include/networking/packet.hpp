#pragma once

#include <string>
#include <cstdint>
#include <sstream>

namespace Networking
{
    enum class PacketType : uint16_t
    {
        STR = 0x0000,
        // DATA = 0x0001,
        // CMD = 0x0002,
        // ACK = 0x0003,
        // NACK = 0x0004,
    };

    class Packet
    {
    private:
        PacketType type;
        uint16_t seqn;
        uint16_t length;
        uint16_t timestamp;
        std::string payload;

    public:
        Packet() : type(PacketType::STR), seqn(0), length(0), timestamp(0), payload("") {}
        Packet(PacketType type, uint16_t seqn, uint16_t timestamp, const std::string &payload) : type(type), seqn(seqn), length(payload.size()), timestamp(timestamp), payload(payload) {}
        std::string serialize() const;
        void deserialize(const std::string &data);
        PacketType getType() const { return type; }
        uint16_t getSeqn() const { return seqn; }
        uint16_t getLength() const { return length; }
        uint16_t getTimestamp() const { return timestamp; }
        std::string getPayload() const { return payload; }
    };
}