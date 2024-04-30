#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <variant>
#include <tuple>

#include "common/pcinfo.hpp"
#include "networking/addresses/mac.hpp"

namespace Networking::Packets
{

    enum class PacketType : uint16_t
    {
        DATA = 0x0000,    // Unkown Data
        SSR = 0x0001,     // Sleep Status Request
        SSR_ACK = 0x0002, // Sleep Status Request Acknowledgement
        STR = 0x0003,     // String
        SSE = 0x0004,     // Sleep Service Exit
        SSD = 0x0005,     // Sleep Service Discovery
        SSD_ACK = 0x0006, // Sleep Service Discovery Acknowledgement
        MAGIC = 0xFFFF,   // Magic Packet
    };

    typedef std::vector<uint8_t> payload_t;

    typedef std::pair<PC::hostname_t, Addresses::Mac> SSE_Data;

    typedef std::variant<std::string, payload_t, SSE_Data, Addresses::Mac> body_t;

    template <typename T>
    inline void to_bytes(payload_t &vec, const T &it, const size_t size)
    {
        vec.insert(vec.end(), it, it + size);
    }

    template <typename T>
    inline void to_bytes(payload_t &vec, const T &begin, const T &end)
    {
        vec.insert(vec.end(), begin, end);
    }

    template <typename T>
    inline void to_bytes(payload_t &vec, const T &data)
    {
        constexpr auto size = sizeof(T) / sizeof(uint8_t);

        for (size_t i = 0; i < size; i++)
        {
            vec.push_back((data >> (i * 8)) & 0xFF);
        }
    }

    template <typename T>
    T from_bytes(const payload_t::const_iterator &begin) noexcept
    {
        constexpr auto size = sizeof(T) / sizeof(uint8_t);
        constexpr auto max_size = sizeof(uint64_t) / sizeof(uint8_t);
        static_assert(size <= max_size, "Type is too large to be deserialized");
        uint64_t value = 0;
        // Value is stored in little-endian
        for (auto it = begin + size - 1; it >= begin; it--)
        {
            value = (value << 8) | *it;
        }

        return static_cast<T>(value);
    }

    typedef uint16_t magic_number_t;

    constexpr magic_number_t MAGIC_NUMBER = 0xCA31;

    inline bool checkMagicNumber(const payload_t &data)
    {
        return *reinterpret_cast<const magic_number_t *>(data.data()) == MAGIC_NUMBER;
    }
}