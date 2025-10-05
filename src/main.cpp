/**
 * @file main.cpp
 * @brief Entry point of the ISA DNS filtering resolver.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-04
 */
#include "../include/errors.h"
#include "../include/tools.h"
#include "../include/sig.h"

#include <iostream>
#include <cstdint>
#include <string>
#include <limits>
#include <set>

void print_help() {
    std::cout << "Filtering DNS resolver - forwards DNS A-type queries except those blocked by list.\n\n";
    std::cout << "Usage: ./dns -s <hostname|ip> [-p port] [-f filter_file] [-v] [-h]\n\n"
    << "Options:\n"
    << "  -s <server>    Set resolver IP address or hostname. Required.\n"
    << "  -p <port>      Listening port (default: 53).\n"
    << "  -f <file>      File with blocked domains.\n"
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

int main (int argc, char **argv) {
    setup_signal_handlers();
    add_cleanup(free_stuff);
    
    std::string resolver_ip;
    std::string filter_file;
    uint16_t port = 53;
    bool verbose = false;

    std::set<std::string> arg_flags =  {
        "-s", // DNS resolver address
        "-p", // listening port - default 53
        "-f", // file with blocked domains
        "-v", // verbose logging
        "-h"  // prints help
    };

    auto get_next_arg = [&](int &i, const std::string &flag, const std::string &def="") 
    -> std::string {
        if (i + 1 < argc && !arg_flags.contains(argv[i+1]))  {
            return argv[++i];
        }

        if(!def.empty()) return def;

        printf_debug("Error: Missing argument for %s\n", flag.c_str());
        (void)flag; // unused-variable

        exit(ERR_BAD_INPUT);
    };

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-s") {
            resolver_ip = get_next_arg(i, arg);
        }
        else if (arg == "-p") {
            port = catch_stoi(
                    get_next_arg(i, arg, "53"), std::numeric_limits<uint16_t>::max(),arg);
            (void) port;
        }
        else if (arg == "-f") {
            filter_file = get_next_arg(i, arg);
        }
        else if (arg == "-v") {
            verbose = true;
            (void) verbose;
        }
        else if (arg == "-h") {
            print_help();
        }
        else {
            // ignore, probably an user typo
        }
    }

    while(!stop_request) {
        // main loop
    }
    graceful_exit();
    printf_debug("Finished");
    return 0;
}