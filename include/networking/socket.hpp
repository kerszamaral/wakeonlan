#pragma once

#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
constexpr int Socket_Error = SOCKET_ERROR;
constexpr int Invalid_socket = INVALID_SOCKET;
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
typedef int socket_t;
constexpr int Socket_Error = -1;
constexpr int Invalid_Socket = -1;
#endif

// Parts found in
// https://stackoverflow.com/questions/3509011/socket-programming-in-c

class Socket
{
private:
    socket_t sock;
    bool open = false;

    static int close(socket_t soc);
    void checkOpen() const;

public:
    static void initialize();
    static void cleanup();
    static void error(std::string message);
    static void set_saddr(sockaddr_in &addr, const uint32_t &new_addr);
    Socket(socket_t s);
    Socket();
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
