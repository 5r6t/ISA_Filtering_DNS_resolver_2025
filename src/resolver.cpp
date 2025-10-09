/**
 * @file resolver.cpp
 * @brief Manages communication with the upstream DNS resolver, including hostname/IP resolution and query forwarding.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#include "resolver.h"
#include "tools.h"
#include "errors.h"

#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> // getaddrinfo

sockaddr_in resolve_ip(const std::string &host_name, uint16_t port) {

    struct addrinfo hints{}, *result = nullptr; // list of addresses and result of getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    if (getaddrinfo(host_name.c_str(), nullptr, &hints, &result) != 0) {
        std::cerr << "ERROR: Unable to resolve domain name: " << host_name << "\n";
        exit(ERR_INTERNAL);
    }

    // first AF_INET address is kept
    struct sockaddr_in addr{};
    for (auto next = result; next != nullptr; next = next->ai_next) {
        if (next->ai_family == AF_INET) {
            addr = *reinterpret_cast<sockaddr_in*>(next->ai_addr);
            addr.sin_port = htons(port); // add port
            break; // Found address
        }
    }
    char ip_buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip_buf, sizeof(ip_buf));
    printf_debug("Resolved %s -> %s:%d", host_name.c_str(), ip_buf, ntohs(addr.sin_port));
    freeaddrinfo(result);

    return addr; // return address 
}