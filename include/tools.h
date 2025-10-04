/**
 * @file tools.h
 * @brief Header file with function declarations, macros used in ISA DNS project.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-04
 */

#pragma once

#include <string>

// THIS DEBUG MACRO HAS BEEN COPPIED FROM
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

// Parses string into int, validates range <0, max_val>
int catch_stoi(const std::string &str, const int max_val, const std::string &varname);
