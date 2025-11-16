/**
 * @file filter.h
 * @brief Domain filtering functions
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#pragma once
#include <string>
#include <unordered_set>

bool is_blocked (const std::unordered_set<std::string>& blocklist, std::string& domain);
std::unordered_set<std::string> filter_load(const std::string& filter_file);
