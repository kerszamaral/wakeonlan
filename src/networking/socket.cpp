#include "networking/socket.hpp"

#ifdef _WIN32
bool Socket::wsaInit = false;
WSADATA Socket::wsaData = WSADATA();

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

#else

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
#endif

void Socket::checkOpen() const
{
    if (!open)
    {
        throw std::runtime_error("Socket is not open");
    }
}

Socket::Socket(SOCKET s)
{
    Socket::initialize();
    sock = s;
    open = true;
}

Socket::Socket()
{
    Socket::initialize();
    sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
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
    if (bytes_sent == SOCKET_ERROR)
    {
        Socket::error("send failed");
    }
}

std::string Socket::receive() const
{
    checkOpen();
    std::string buffer(4096, 0);
    auto bytes_received = ::recv(sock, buffer.data(), buffer.length(), 0);
    if (bytes_received == SOCKET_ERROR)
    {
        Socket::error("recv failed");
    }
    return buffer;
}

void Socket::setOpt(const int &level, const int &optname, const int &optval)
{
    checkOpen();
    auto setsockresult = ::setsockopt(sock, level, optname, (char *)&optval, sizeof(optval));
    if (setsockresult == SOCKET_ERROR)
    {
        Socket::error("setsockopt failed");
    }
}

void Socket::bind(const sockaddr_in &addr)
{
    checkOpen();
    auto bind_result = ::bind(sock, (sockaddr *)&addr, sizeof(addr));
    if (bind_result == SOCKET_ERROR)
    {
        Socket::error("bind failed");
    }
}

void Socket::listen(const int &backlog)
{
    checkOpen();
    auto listen_result = ::listen(sock, backlog);
    if (listen_result == SOCKET_ERROR)
    {
        Socket::error("listen failed");
    }
}

SOCKET Socket::accept(sockaddr_in &addr)
{
    checkOpen();
    auto addr_len = sizeof(struct sockaddr_in);
    SOCKET client_socket = ::accept(sock, (sockaddr *)&addr, (socklen_t *)&addr_len);
    if (client_socket == INVALID_SOCKET)
    {
        Socket::error("accept failed");
    }
    return client_socket;
}

void Socket::connect(const sockaddr_in &addr)
{
    checkOpen();
    auto connect_result = ::connect(sock, (sockaddr *)&addr, sizeof(addr));
    if (connect_result == SOCKET_ERROR)
    {
        Socket::error("connect failed");
    }
}

void Socket::close()
{
    if (open)
    {
        auto closesocket_result = ::closesocket(sock);
        if (closesocket_result == SOCKET_ERROR)
        {
            Socket::error("closesocket failed");
        }
    }
}
