/**
 * @file udp.h
 * @brief Handles UDP socket creation, binding, and packet send/receive operations.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

 #pragma once

int create_udp_socket();
void bind_udp_socket(int sock, int port);
void sock_close(int *sock);