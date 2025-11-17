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
#include <unordered_map>

#include <arpa/inet.h>
#include <sys/socket.h>

using enum DNSError;

// Time to drop old entries
constexpr int WAITTIME_S = 5; 
constexpr int WAITTIME_U = 0;

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

    if (parse_id(pkt.data, dmsg.head.id));
    else dmsg.head.id = 0;
    
    // Malformed check: DNS header must be at least 12 bytes
    if (pkt.data.size() < 12) {
        logv("Received packet is malformed (contains less than 12bytes).");
        auto resp = build_error(dmsg, FORMAT_ERR);
        udp_send(cfg.sock_local, resp, pkt.src);
        logv("Sent FORMAT_ERROR response to client.");
        return;
    }
    // Parse check
    if (!parse_dns_q(pkt.data, dmsg, offset)) {
        logv("Message couldn't be parsed.");
        auto response = build_error(dmsg, FORMAT_ERR);
        udp_send(cfg.sock_local, response, pkt.src);
        logv("Sent FORMAT_ERR response to client.");
        return;
    }
    // Unsupported query
    else if (!is_A_query(dmsg.que.qtype)) {
        logv("Messages other than A querries are unsupported.");
        auto response = build_error(dmsg, NOT_IMPLEM_ERR);
        udp_send(cfg.sock_local, response, pkt.src);
        logv("Sent NOT_IMPLEMENTED_ERROR response to client.");
        return;
    }
    // Blocked domain
    else if (is_blocked(blocklist, dmsg.que.qname)) {
        logv("Requested domain '%s' is blocked.", dmsg.que.qname.c_str());
        auto response = build_error(dmsg, REFUSED_ERR);
        udp_send(cfg.sock_local, response, pkt.src);
        logv("Sent REFUSED_ERROR response to client.");
        return;
    }
    else {
        logv("Domain %s is allowed.", dmsg.que.qname.c_str());
        table[dmsg.head.id] = pkt.src; // store for later
        logv("Stored mapping for id=%u", dmsg.head.id);
        udp_send(cfg.sock_upstream, pkt.data, cfg.r_addr);
        logv("Forwarded message to upstream resolver.");
    }
}


/// @brief Handles DNS replies received from the upstream resolver.
/// @param table Map of DNS IDs to client addresses.
/// @param pkt   Received UDP packet.
void handle_upstream(std::unordered_map<uint16_t, sockaddr_storage> &table,
                        const UdpPacket &pkt)
{
    uint16_t id;
    if (!parse_id(pkt.data, id)) {
        logv("Received malformed packet from the upstream resolver (message dropped).");
        return; // drop packet
    }
    auto it = table.find(id);
    if (it == table.end()) {
        logv("No mapping found for id=%u (message dropped)", id);
        return; // drop packet
    }


    udp_send(cfg.sock_local, pkt.data, it->second);
    logv("Forwarded reply id=%u to client", id);
    table.erase(it); // clear the record
    logv("Record cleared.");
}

/// @brief Main event loop: receives queries, forwards them, and returns responses.
void runtime() {
    // LOCAL SOCKET: always IPv6 dual-stack
    cfg.sock_local = create_udp_socket(AF_INET6);
    bind_udp_socket(cfg.sock_local, cfg.loc_port, AF_INET6);
    logv("Client socket bound");

    // UPSTREAM SOCKET: match upstream family
    int up_family = cfg.r_addr.ss_family;
    cfg.sock_upstream = create_udp_socket(up_family);

    std::unordered_set<std::string> blocklist = filter_load(cfg.filter_file);
    logv("Blocklist loaded from %s", cfg.filter_file.c_str());
    std::unordered_map<uint16_t, sockaddr_storage> table;

    fd_set readfds; // set of file descriptors
    setup_signal_handlers();
    
    logv("Listening on port %d, upstream \"%s\" socket ready", cfg.loc_port, cfg.hostname.c_str());
    logv("Starting runtime...");
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
                logv("Select interrupted by signal (possible SIGINT).");
                continue;
            }
            logv("Select error (errno=%d), shutting down.", errno);
            break;
        }

        // drop queries after 5 seconds of inactivity, avoids stale entries (e.g. from dropped packets)
        if (ready == 0 && !table.empty()) 
        {
            logv("Timeout: Query table dropped");
            table.clear();
        }

        if (FD_ISSET(cfg.sock_local, &readfds)) 
        {
            logv("Data received from client.");
            UdpPacket pkt = udp_receive(cfg.sock_local);
            if (!pkt.data.empty())
            {
                handle_client(blocklist, table, pkt);  
            }
        } // client

        if (FD_ISSET(cfg.sock_upstream, &readfds)) 
        {
            logv("Data received from upstream resolver.");
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
    cfg.r_addr = resolve_host(cfg.hostname);
    logv("Resolved %s to %s", cfg.hostname.c_str() ,addr_to_str(cfg.r_addr));
    runtime();
    return 0;
}