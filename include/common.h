/**
 * @file tools.h
 * @brief Utility functions and macros for ISA DNS project.
 * 
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-04
 */

#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include <netinet/in.h>

// THIS DEBUG MACRO HAS BEEN COPIED FROM
// https://git.fit.vutbr.cz/NESFIT/IPK-Projects/src/branch/master/Project_2#client-logging
#ifdef DEBUG_PRINT
#define printf_debug(format, ...) \
    do { \
        if (*#format) { \
            fprintf(stderr, "%s:%-4d | %15s | " format "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
        } \
        else { \
            fprintf(stderr, "%s:%-4d | %15s | \n", __FILE__, __LINE__, __func__); \
        } \
    } while (0)
#else
#define printf_debug(format, ...) ((void)0)
#endif

#define logv(fmt, ...) \
    do { if (cfg.verbose) fprintf(stdout, "|VERB|\t" fmt "\n", ##__VA_ARGS__); } while (0)


struct Config {
    uint16_t loc_port = 53;

    int sock_local = -1;
    int sock_upstream = -1;
    bool verbose = false;

    std::string hostname;
    std::string filter_file;

    sockaddr_storage r_addr{};
};

int catch_stoi(const std::string &str, const int max_val, const std::string &varname);
void append_u8(std::vector<uint8_t>& buf, uint8_t value);
void append_u16(std::vector<uint8_t>& buf, uint16_t value);
void append_u32(std::vector<uint8_t>& buf, uint32_t value);
void append_dns_name(std::vector<uint8_t>& buf, const std::string& domain);
uint16_t read_u16(const std::vector<uint8_t>& buf, size_t offset);
uint32_t read_u32(const std::vector<uint8_t>& buf, size_t offset);