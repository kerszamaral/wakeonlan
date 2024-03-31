#include "networking/sockets/tcp.hpp"

namespace Networking::Sockets
{
    TCP::TCP(const Networking::Addresses::Address &addr) : Socket(Type::TCP)
    {
        connect(addr.getAddr());
    }

    TCP::~TCP()
    {
        close();
    }
}