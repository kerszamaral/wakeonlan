#pragma once

#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
typedef int socket_t;
#endif

// Parts found in
// https://stackoverflow.com/questions/3509011/socket-programming-in-c

class Socket
{
private:
    socket_t sock;
    bool open = false;
    constexpr static size_t BUFFER_SIZE = 4096;

#ifdef _WIN32
    constexpr static int ERROR = SOCKET_ERROR;
    constexpr static int INVALID = INVALID_SOCKET;
#else
    constexpr static int ERROR = -1;
    constexpr static int INVALID = -1;
#endif

    static int close(socket_t soc);
    void checkOpen() const;

public:
    enum class Type : int
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    enum class Protocol : int
    {
        TCP = IPPROTO_TCP,
        UDP = IPPROTO_UDP
    };

    enum class IPVersion : int
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6
    };

    Socket(socket_t s);
    Socket(Socket::Type type);
    ~Socket();
    void send(std::string message) const;
    std::string receive() const;
    socket_t getSocket() { return sock; }
    void setOpt(const int &level, const int &optname, const int &optval);
    void bind(const sockaddr_in &addr);
    void listen(const int &backlog);
    socket_t accept(sockaddr_in &addr);
    void connect(const sockaddr_in &addr);
    void close();
    static void initialize();
    static void cleanup();
    static void error(std::string message);
    static void set_saddr(sockaddr_in &addr, const uint32_t &new_addr);

    friend std::ostream &operator<<(std::ostream &os, const Socket &s)
    {
        return os << s.receive();
    }
    friend std::istream &operator>>(std::istream &is, const Socket &s)
    {
        std::string message;
        is >> message;
        s.send(message);
        return is;
    }

#ifdef _WIN32
private:
    static WSADATA wsaData;
    static bool wsaInit;

public:
    static bool getWsaInit() { return wsaInit; }
#endif
};
