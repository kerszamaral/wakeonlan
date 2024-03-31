#include "networking/sockets/udp.hpp"

namespace Networking::Sockets
{
    UDP::UDP(const Networking::Addresses::Address &address) : Socket(Type::UDP)
    {
        addr = address;
    }

    UDP::~UDP()
    {
        close();
    }
}