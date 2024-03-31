#include "networking/sockets/listener.hpp"

namespace Networking::Sockets
{
    PortListener::PortListener(Networking::Addresses::Port port)
    {
        addr.setPort(port);

        server.setOpt(SOL_SOCKET, SO_REUSEADDR, opt);

        server.bind(addr);

        constexpr auto backlog = SOMAXCONN;
        server.listen(backlog);
    }

    TCP PortListener::waitForConnection()
    {
        return TCP(server.accept(addr));
    }

    PortListener::~PortListener()
    {
        // server.close();
    }
}