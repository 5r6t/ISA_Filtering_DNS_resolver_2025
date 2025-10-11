/**
 * @file udp.cpp
 * @brief Handles UDP socket creation, binding, and packet send/receive operations.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#include "udp.h"
#include "common.h"
#include "errors.h"

#include <iostream>
#include <cstring>

#include <netinet/in.h>
#include <unistd.h>


void sock_close(int *sock) {
    if (*sock != -1) {
        close(*sock);
        *sock = -1;
    }
}

int create_udp_socket(int family) 
{
    int sock = socket(family, SOCK_DGRAM, 0);
    
    if (sock == -1) {
        std::cerr << "ERROR: Cannot create socket.\n";
        exit(ERR_INTERNAL);
    }

    if (family == AF_INET6) {
        int off = 0; // allow both v4 and v6
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &off, sizeof(off));
    }
    
    return sock;
}

void bind_udp_socket(int sock, int port, int family)
{
    if (family == AF_INET) {
        struct sockaddr_in s_addr{};

        s_addr.sin_family = AF_INET; 
        s_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
        s_addr.sin_port = htons(port); 

        int addr_size = sizeof(s_addr); 
        const sockaddr* addr = reinterpret_cast<sockaddr*>(&s_addr);

        if (bind(sock, addr, addr_size) < 0) 
        { 
            std::cerr << "ERROR: Cannot bind IPv4 socket.\n";
            exit(EXIT_FAILURE);
        }
    } 
    else if (family == AF_INET6) {
        struct sockaddr_in6 s_addr6{};

        s_addr6.sin6_family = AF_INET6; 
        s_addr6.sin6_addr = in6addr_any; 
        s_addr6.sin6_port = htons(port);

        int addr6_size = sizeof(s_addr6);
        const sockaddr* addr6 = reinterpret_cast<sockaddr*>(&s_addr6);

        if (bind(sock, addr6, addr6_size) < 0) 
        { 
            std::cerr << "ERROR: Cannot bind IPv6 socket.\n";
            exit(EXIT_FAILURE);
        }
    }
    else {
        std::cerr << "ERROR: Unsupported address family for binding.\n";
        exit(ERR_INTERNAL);
    }
    printf_debug("Socket bound to port %d (family %s)", port,
                  (family == AF_INET ? "IPv4" : "IPv6"));
}