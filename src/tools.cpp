/**
 * @file tools.cpp
 * @brief Contains utility functions for ISA DNS project.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-04
 */
#include "../include/tools.h"
#include "../include/errors.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>

int catch_stoi(const std::string &str, const int max_val, const std::string &varname)
{
    try {
        size_t idx;
        int val = std::stoi(str, &idx);
        if (idx != str.size() || val < 0 || val > max_val)
            throw std::out_of_range("Invalid range");
        return val;
    } catch (...) {
        printf_debug("Error: %s must be in range <0,%d>", varname.c_str(), max_val);
        exit(ERR_BAD_INPUT);
    }
}
