/**
 * @file sig.h
 * @brief Handles signal setup and graceful program termination (e.g., SIGINT cleanup).
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

// Add a cleanup function to be execute on graceful exit.
// Each function must have signature: void func(void)
void add_cleanup(void (*fn)());

// Executes all registered cleanup functions and exits with the given code.
void graceful_exit(int code = 0);