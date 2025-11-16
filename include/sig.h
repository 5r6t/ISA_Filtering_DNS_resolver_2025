/**
 * @file sig.h
 * @brief Handles signal setup
 * 
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-05
 */

#pragma once
#include <atomic>

extern std::atomic<bool> stop_request;

// Register the SIGINT handler.
void setup_signal_handlers();