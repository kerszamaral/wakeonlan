#include "networking/tcp.hpp"

namespace Socket
{
    TCP::TCP(const Address &addr) : Socket(Type::TCP)
    {
        connect(addr.getAddr());
    }

    TCP::~TCP()
    {
        close();
    }
}