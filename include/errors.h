/**
 * @file errors.h
 * @brief Error definitions
 *
 * @author Jaroslav Mervart
 * @login xmervaj00
 * @date 2025-10-04
 */
#pragma once

#define OK               0
#define ERR_ARGS         1 //
#define ERR_FILE         2 //
#define ERR_BAD_INPUT    3 //

#define ERR_SOCK_CREATE  10
#define ERR_SOCK_BIND    11 //
#define ERR_SOCK_RECV    12
#define ERR_SOCK_SEND    13

#define ERR_DNS_PARSE    20
#define ERR_DNS_BUILD    21

#define ERR_INTERNAL     99 //