#include "networking/socket.hpp"

#ifdef _WIN32
bool Socket::wsaInit = false;
WSADATA Socket::wsaData = WSADATA();

int Socket::close(socket_t soc)
{
    return ::closesocket(soc);
}

void Socket::initialize()
{
    if (!Socket::getWsaInit())
    {
        int iResult = WSAStartup(MAKEWORD(2, 2), &Socket::wsaData);
        if (iResult != 0)
        {
            throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
        }
        Socket::wsaInit = true;
    }
}

void Socket::cleanup()
{
    if (Socket::getWsaInit())
    {
        WSACleanup();
        Socket::wsaInit = false;
    }
}

void Socket::error(std::string message)
{
    throw std::runtime_error(message + ": " + std::to_string(WSAGetLastError()));
}

void Socket::set_saddr(sockaddr_in &addr, const uint32_t &new_addr)
{
    addr.sin_addr.S_un.S_addr = new_addr;
}

#else

int Socket::close(socket_t soc)
{
    return ::close(soc);
}

void Socket::initialize()
{
}

void Socket::cleanup()
{
}

void Socket::error(std::string message)
{
    throw std::runtime_error(message);
}

void Socket::set_saddr(sockaddr_in &addr, const uint32_t &new_addr)
{
    addr.sin_addr.s_addr = new_addr;
}
#endif

void Socket::checkOpen() const
{
    if (!open)
    {
        throw std::runtime_error("Socket is not open");
    }
}

Socket::Socket(socket_t s)
{
    Socket::initialize();
    sock = s;
    open = true;
}

Socket::Socket()
{
    Socket::initialize();
    sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock == Invalid_Socket)
    {
        Socket::error("socket failed");
    }
    open = true;
}

Socket::~Socket()
{
}

void Socket::send(std::string message) const
{
    checkOpen();
    auto bytes_sent = ::send(sock, message.c_str(), message.length(), 0);
    if (bytes_sent == Socket_Error)
    {
        Socket::error("send failed");
    }
}

std::string Socket::receive() const
{
    checkOpen();
    std::string buffer(4096, 0);
    auto bytes_received = ::recv(sock, buffer.data(), buffer.length(), 0);
    if (bytes_received == Socket_Error)
    {
        Socket::error("recv failed");
    }
    return buffer;
}

void Socket::setOpt(const int &level, const int &optname, const int &optval)
{
    checkOpen();
    auto setsockresult = ::setsockopt(sock, level, optname, (char *)&optval, sizeof(optval));
    if (setsockresult == Socket_Error)
    {
        Socket::error("setsockopt failed");
    }
}

void Socket::bind(const sockaddr_in &addr)
{
    checkOpen();
    auto bind_result = ::bind(sock, (sockaddr *)&addr, sizeof(addr));
    if (bind_result == Socket_Error)
    {
        Socket::error("bind failed");
    }
}

void Socket::listen(const int &backlog)
{
    checkOpen();
    auto listen_result = ::listen(sock, backlog);
    if (listen_result == Socket_Error)
    {
        Socket::error("listen failed");
    }
}

socket_t Socket::accept(sockaddr_in &addr)
{
    checkOpen();
    auto addr_len = sizeof(struct sockaddr_in);
    socket_t client_socket = ::accept(sock, (sockaddr *)&addr, (socklen_t *)&addr_len);
    if (client_socket == Invalid_Socket)
    {
        Socket::error("accept failed");
    }
    return client_socket;
}

void Socket::connect(const sockaddr_in &addr)
{
    checkOpen();
    auto connect_result = ::connect(sock, (sockaddr *)&addr, sizeof(addr));
    if (connect_result == Socket_Error)
    {
        Socket::error("connect failed");
    }
}

void Socket::close()
{
    if (open)
    {
        auto closesocket_result = Socket::close(sock);
        if (closesocket_result == Socket_Error)
        {
            Socket::error("closesocket failed");
        }
    }
}
