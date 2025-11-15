/**
 * @file udp.h
 * @brief Handles UDP socket creation, binding, and packet send/receive operations.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#pragma once

#include <initializer_list>
#include <vector>
#include <cstdint>

#include <netinet/in.h>

struct UdpPacket {
    std::vector<uint8_t> data;
    sockaddr_storage src;
    socklen_t len;
};

int create_udp_socket(int family);
void bind_udp_socket(int sock, int port, int family);
void sock_close(std::initializer_list<int*> socks);
UdpPacket udp_receive(const int sock);
void udp_send (const int sock, const std::vector<uint8_t>& data, const sockaddr_storage& addr);