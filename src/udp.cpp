/**
 * @file udp.cpp
 * @brief Handles UDP socket creation, binding, and packet send/receive operations.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#include "udp.h"
#include "tools.h"
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

int create_udp_socket() 
{
    // family - IPv4, type - UDP, protocol - use default
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        std::cerr << "ERROR: Cannot create socket.\n";
        exit(ERR_INTERNAL);
    }
    return sock;
}

void bind_udp_socket(int sock, int port)
{
    struct sockaddr_in s_addr{}; // initialize the struct

    s_addr.sin_family = AF_INET; 
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    s_addr.sin_port = htons(port); 

    int addr_size = sizeof(s_addr); 
    const sockaddr* addr = reinterpret_cast<sockaddr*>(&s_addr);

    if (bind(sock, addr, addr_size) < 0) 
    { 
        perror("ERROR: Can't bind to socket");
        std::cerr << "ERROR: Cannot bind socket.\n";
        exit(EXIT_FAILURE);
    }
    printf_debug("Socket bound to port %d", port);
}