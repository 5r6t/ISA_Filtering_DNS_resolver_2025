/**
 * @file filter.cpp
 * @brief Implements domain filtering logic based on the blocklist file and its subdomains.
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#include "errors.h"
#include "common.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <unordered_set>

// lowercase string conversion adapted from:
// https://stackoverflow.com/a/313990
void normalize (std::string &line) {
    std::transform(line.begin(), line.end(), line.begin(),
        [](unsigned char c){ return std::tolower(c); });
    // careful about trimming empty lines -- is_skippable()
    line.erase(0, line.find_first_not_of(" \t\r\n")); // left
    line.erase(line.find_last_not_of(" \t\r\n") + 1); // right
}

bool is_valid_domain (std::string line) {
    
    printf_debug("%s", line.c_str());
    (void)line;
    return 1;
}

bool is_skippable(std::string line) {
    return line.empty() || line[0] == '#';
}

std::unordered_set<std::string> filter_load(std::string filter_file) 
{
    std::ifstream file(filter_file);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file\n";
        exit(ERR_FILE);
    }

    std::unordered_set<std::string> blocked;
    std::string line;

    while (std::getline(file, line)) {
        // sort and fix and clear up name
        if (is_skippable(line)) 
            continue;

        normalize(line);

        if (!is_valid_domain(line)) {
            ;
        }
        blocked.insert(line);
        
    }

    file.close();
    printf_debug("Filter list updated");
    return blocked;
}