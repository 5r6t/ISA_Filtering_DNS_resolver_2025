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

#include <iostream>
#include <cstdint>
#include <string>
#include <limits>
#include <set>

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
    exit(0);
}

void free_stuff() {
    printf_debug("Freeing non-existent resources...\n");
}

void runtime(Config &cfg) {
    // main runtime loop
    int family = cfg.r_addr.ss_family;
    cfg.sock = create_udp_socket(family);
    bind_udp_socket(cfg.sock, cfg.r_port, family);

    std::string line = "          ";
    normalize(line);
    if (is_skippable(line)) printf_debug("Skipped");
    else printf_debug("%s|\n", line.c_str());

    while(!stop_request) {
        // main loop

    }
    sock_close(&cfg.sock);
}


int main (int argc, char **argv) {
    Config cfg;
    setup_signal_handlers();
    add_cleanup(free_stuff);

    std::set<std::string> arg_flags =  {
        "-s", // DNS resolver address
        "-p", // listening port - default 53
        "-f", // file with blocked domains
        "-v", // verbose logging
        "-h"  // prints help
    };
    
    // lambda function to get next argument
    auto get_next_arg = [&](int &i, const std::string &flag, const std::string &def="") 
    -> std::string {

        if (i + 1 < argc && !arg_flags.contains(argv[i+1]))  {
            return argv[++i];
        }

        if(!def.empty()) return def;
        std::cerr << "Error: Missing argument for" << flag << "\n";

        exit(ERR_BAD_INPUT);
    };

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-s") {
            cfg.hostname = get_next_arg(i, arg);
        }
        else if (arg == "-p") {
            int maxval = std::numeric_limits<uint16_t>::max();
            cfg.r_port = catch_stoi(
                    get_next_arg(i, arg, "53"), maxval ,arg);
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
    cfg.r_addr = resolve_host(cfg.hostname, cfg.r_port);
    // similarly, filter file function will error out if file is bad/missing

    runtime(cfg);

    cleanup();
    return 0;
}