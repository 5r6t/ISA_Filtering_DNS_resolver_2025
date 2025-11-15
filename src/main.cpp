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

[[noreturn]] void print_help() {
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
    exit(0);
}

void handle_client(const std::unordered_set<std::string> &blocklist,
                    std::unordered_map<uint16_t, sockaddr_storage> &table,
                    const UdpPacket &pkt)
{
    DnsMsg dmsg{};
    size_t offset = 0;
    if ( !parse_dns_q(pkt.data, dmsg, offset)) {
        auto response = build_error(dmsg, FORMAT_ERR);
        udp_send(cfg.sock_local, response, pkt.src); 
    }
    // handling
    else if ( !is_A_query(dmsg.que.qtype)) {
        auto response = build_error(dmsg, NOT_IMPLEM_ERR);
        udp_send(cfg.sock_local, response, pkt.src);
    }
    else if (is_blocked(blocklist, dmsg.que.qname)) {
        auto response = build_error(dmsg, REFUSED_ERR);
        udp_send(cfg.sock_local, response, pkt.src); 
    }
    else {
        table[dmsg.head.id] = pkt.src; // store for later
        printf_debug("Stored mapping id=%u", dmsg.head.id);
        udp_send(cfg.sock_upstream, pkt.data, cfg.r_addr);
    }
}

// assumes queries from upstream resolver are correct
void handle_upstream(std::unordered_map<uint16_t, sockaddr_storage> &table,
                        const UdpPacket &pkt)
{
    DnsMsg dmsg{};
    size_t offset = 0;
    parse_dns_q(pkt.data, dmsg, offset);
    parse_dns_a(pkt.data, dmsg, offset);

    auto it = table.find(dmsg.head.id);
    if (it == table.end()) {
        printf_debug("No mapping found for id=%u (reply dropped)", dmsg.head.id);
        return; // drop packet
    }
    if (!is_A_query(dmsg.que.qtype)) return; // drop packet

    if (!is_error(dmsg.head.flags)) printf_debug("Reply is an error");
    printf_debug("Forwarding reply id=%u to client", dmsg.head.id);

    udp_send(cfg.sock_local, pkt.data, it->second);
    table.erase(it); // clear the record
}

void runtime() {
    int family = cfg.r_addr.ss_family;
    cfg.sock_local = create_udp_socket(family);
    cfg.sock_upstream = create_udp_socket(family);
    // bind just the local socket
    bind_udp_socket(cfg.sock_local, cfg.loc_port, family);
    
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
            perror("Select");
            break;
        }

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
        }
        else {
            
            // ignore, probably an user typo
        }
    }
    cfg.r_addr = resolve_host(cfg.hostname);
    runtime();

    return 0;
}