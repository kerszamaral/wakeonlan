#include "networking/udp.hpp"

namespace Socket
{
    UDP::UDP(const Address &address) : Socket(Type::UDP)
    {
        addr = address;
    }

    UDP::~UDP()
    {
        close();
    }
}