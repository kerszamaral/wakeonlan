#pragma once

#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

// Parts found in
// https://stackoverflow.com/questions/3509011/socket-programming-in-c

class Socket
{
private:
    SOCKET sock;
    bool open = false;

    void checkOpen();

public:
    static void initialize();
    static void cleanup();
    static void error(std::string message);
    Socket(SOCKET s);
    Socket();
    ~Socket();
    void send(std::string message);
    std::string receive();
    SOCKET getSocket() { return sock; }
    void setOpt(const int &level, const int &optname, const int &optval);
    void bind(const sockaddr_in &addr);
    void listen(const int &backlog);
    SOCKET accept(sockaddr_in &addr);
    void connect(const sockaddr_in &addr);
    void close();

#ifdef _WIN32
private:
    static WSADATA wsaData;
    static bool wsaInit;

public:
    static bool getWsaInit() { return wsaInit; }
#endif
};
