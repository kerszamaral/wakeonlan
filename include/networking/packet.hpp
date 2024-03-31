#pragma once

#include <cstdint>
namespace Networking
{
    enum PacketType : uint16_t
    {
        DATA = 0x0001,
        CMD = 0x0002,
        ACK = 0x0003,
        NACK = 0x0004,
    };

    typedef struct
    {
        PacketType type;      // Packet type (e.g. DATA | CMD)
        uint16_t seqn;        // Sequence number
        uint16_t length;      // Payload length
        uint16_t timestamp;   // Data timestamp
        const char *_payload; // Message data
    } Packet;
}