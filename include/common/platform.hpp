#pragma once

#if defined(_WIN32)
#define OS_WIN
#elif defined(__GNUG__) || defined(__GNUC__)
#define OS_LINUX
#elif defined(_MAC)
#define OS_MAC
#endif

#ifdef OS_WIN
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <codecvt>
#include <locale>
#elif defined(OS_LINUX) || defined(OS_MAC)
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#endif