#include "networking/listener.hpp"

PortListener::PortListener(Port port)
{
    server.setOpt(SOL_SOCKET, SO_REUSEADDR, opt);

    addr.sin_family = AF_INET;
    Socket::set_saddr(addr, INADDR_ANY);
    addr.sin_port = port.get_port();

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