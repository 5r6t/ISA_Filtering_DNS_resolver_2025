/**
 * @file main.cpp
 * @brief Entry point of the ISA DNS filtering resolver.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-04
 */

#include "errors.h"
#include "common.h"
#include "sig.h"
#include "resolver.h"
#include "udp.h"
#include "filter.h"
#include "dns_parser.h"

#include <iostream>
#include <cstdint>
#include <string>
#include <limits>
#include <set>
#include <arpa/inet.h>
#include <cstdint>
#include <unordered_map>

constexpr int WAITTIME_S = 5; // time to drop old entries
constexpr int WAITTIME_U = 0;
using enum DNSError;

Config cfg;

/// @brief Prints usage information and command-line help.
void print_help() {
    std::cout << "Filtering DNS resolver - forwards DNS A-type queries except those blocked by list.\n\n";
    std::cout << "Usage: ./dns -s <hostname|ip> [-p port] -f <filter_file> [-v] [-h]\n\n"
    << "Options:\n"
    << "  -s <server>    Set resolver IP address or hostname. Required.\n"
    << "  -f <file>      File with blocked domains. Required.\n"
    << "  -p <port>      Listening port (default: 53).\n"
    << "  -v             Enable verbose logging. \n"
    << "  -h             Show this help message and exit.\n\n"
    << "Examples:\n"
    << "  ./dns -s 8.8.8.8 -f blocklist.txt\n"
    << "  ./dns -s dns.google -p 5300 -f filters.txt -v\n";
}

/// @brief Handles DNS queries received from a client.
/// @param blocklist Set of blocked domains.
/// @param table     Map of DNS IDs to client addresses.
/// @param pkt       Received UDP packet.
void handle_client(const std::unordered_set<std::string> &blocklist,
                    std::unordered_map<uint16_t, sockaddr_storage> &table,
                    const UdpPacket &pkt)
{
    DnsMsg dmsg{};
    size_t offset = 0;
    if (!parse_dns_q(pkt.data, dmsg, offset)) {
        // ensures id field is filled 
        if (!parse_id(pkt.data, dmsg.head.id)) {
            return; // drop packet
        }
        auto response = build_error(dmsg, FORMAT_ERR);
        udp_send(cfg.sock_local, response, pkt.src);
        return;
    }
    // handling
    else if (!is_A_query(dmsg.que.qtype)) {
        auto response = build_error(dmsg, NOT_IMPLEM_ERR);
        udp_send(cfg.sock_local, response, pkt.src);
        return;
    }
    else if (is_blocked(blocklist, dmsg.que.qname)) {
        auto response = build_error(dmsg, REFUSED_ERR);
        udp_send(cfg.sock_local, response, pkt.src); 
        return;
    }
    else {
        table[dmsg.head.id] = pkt.src; // store for later
        printf_debug("Stored mapping id=%u", dmsg.head.id);
        udp_send(cfg.sock_upstream, pkt.data, cfg.r_addr);
    }
}


/// @brief Handles DNS replies received from the upstream resolver.
/// @param table Map of DNS IDs to client addresses.
/// @param pkt   Received UDP packet.
void handle_upstream(std::unordered_map<uint16_t, sockaddr_storage> &table,
                        const UdpPacket &pkt)
{
    uint16_t id;
    if (!parse_id(pkt.data, id))
        return; // drop packet

    auto it = table.find(id);
    if (it == table.end()) {
        printf_debug("No mapping found for id=%u (reply dropped)", id);
        return; // drop packet
    }

    printf_debug("Forwarding reply id=%u to client", id);

    udp_send(cfg.sock_local, pkt.data, it->second);
    table.erase(it); // clear the record
}

/// @brief Main event loop: receives queries, forwards them, and returns responses.
void runtime() {
    // LOCAL SOCKET: always IPv6 dual-stack
    cfg.sock_local = create_udp_socket(AF_INET6);
    bind_udp_socket(cfg.sock_local, cfg.loc_port, AF_INET6);

    // UPSTREAM SOCKET: match upstream family
    int up_family = cfg.r_addr.ss_family;
    cfg.sock_upstream = create_udp_socket(up_family);

    std::unordered_set<std::string> blocklist = filter_load(cfg.filter_file);
    std::unordered_map<uint16_t, sockaddr_storage> table;

    fd_set readfds; // set of file descriptors
    setup_signal_handlers();
    
    printf_debug("Listening on port %d, upstream \"%s\" socket ready", cfg.loc_port, cfg.hostname.c_str());

    while(!stop_request) {
        // clear file descriptor set, set descriptors
        FD_ZERO(&readfds); 
        FD_SET(cfg.sock_local, &readfds);
        FD_SET(cfg.sock_upstream, &readfds);

        struct timeval timeout { WAITTIME_S, WAITTIME_U }; 

        int max_fd = std::max(cfg.sock_local, cfg.sock_upstream);
        int ready = select(max_fd+1, &readfds, nullptr, nullptr, &timeout);

        if (ready < 0) 
        {
            if (errno == EINTR) {
                printf_debug("Select interrupted");
                continue;
            }
            printf_debug("Select interrupted");
            break;
        }

        // drop queries after 5 seconds of inactivity, avoids stale entries (e.g. from dropped packets)
        if (ready == 0 && !table.empty()) 
        {
            printf_debug("Timeout: Query table dropped");
            table.clear();
        }

        if (FD_ISSET(cfg.sock_local, &readfds)) 
        {
            printf_debug("CLIENT QUERY RECEIVED");
            UdpPacket pkt = udp_receive(cfg.sock_local);
            if (!pkt.data.empty())
            {
                handle_client(blocklist, table, pkt);  
            }
        } // client

        if (FD_ISSET(cfg.sock_upstream, &readfds)) 
        {
            printf_debug("REPLY FROM UPSTREAM");
            UdpPacket pkt = udp_receive(cfg.sock_upstream);
            if (!pkt.data.empty())
            {
                handle_upstream(table, pkt);
            }
        } // upstream
    } // while
}

/// @brief Performs cleanup actions on program exit.
void cleanup() {
    sock_close({&cfg.sock_local, &cfg.sock_upstream});
}

int main (int argc, char **argv) {
    atexit(cleanup);

    std::set<std::string, std::less<>> arg_flags =  {
        "-s", // DNS resolver address
        "-p", // listening port - default 53
        "-f", // file with blocked domains
        "-v", // verbose logging
        "-h"  // prints help
    };
    
    // lambda function to get next argument
    auto get_next_arg = [&](int &i, const std::string &flag, const std::string &def="") 
    -> std::string {

        const bool has_next = (i + 1 < argc);
        const bool next_is_flag = has_next && arg_flags.contains(argv[i+1]);
        
        if (has_next && !next_is_flag)  {
            return argv[++i]; // increments counter!
        }


        if(!def.empty()) return def;

        std::cerr << "Error: Missing argument for " << flag << "\n";
        exit(ERR_BAD_INPUT);
    };

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-s") {
            cfg.hostname = get_next_arg(i, arg);
            cfg.r_addr = resolve_host(cfg.hostname);
        }
        else if (arg == "-p") {
            int maxval = std::numeric_limits<uint16_t>::max();
            cfg.loc_port = static_cast<uint16_t>(catch_stoi(
                    get_next_arg(i, arg, "53"), maxval ,arg));
        }
        else if (arg == "-f") {
            cfg.filter_file = get_next_arg(i, arg);
        }
        else if (arg == "-v") {
            cfg.verbose = true;
        }
        else if (arg == "-h") {
            print_help();
            exit(OK);
        }
        else {
            // invalid flag
            std::cerr << "Error: Unknown argument: " << arg << "\n";
            print_help();
            exit(ERR_ARGS);
        }
    }

    // required arguments
    if (cfg.hostname.empty() || cfg.filter_file.empty()) {
        std::cerr << "Error: -s and -f are required.\n";
        print_help();
        return ERR_ARGS;
    }

    runtime();

    return 0;
}