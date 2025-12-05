# ISA - DNS FILTERING RESOLVER
- name  : Jaroslav Mervart
- xlogin: xmervaj00
- date created: 30th of September 2025

# Short description of the program
Filtering DNS resolver that listens on a UDP IPv6 dual-stack socket, 
accepts A-type queries from clients, and forwards them to a configurable 
upstream resolver unless the queried domain (or any of its parent domains) 
is blocked. 
The resolver maps DNS IDs to client addresses, hence forwarding replies 
back to the correct client (see Limitations), supports graceful shutdown via SIGINT and 
offers verbose output through `-v`.

## Extensions
- none

## Prerequisites
- A compiler that supports C++20 (e.g. GCC 15)

## Limitations

- Blocklist is loaded only once at startup. 
  (To reflect updates made during runtime, the resolver needs to restart)
- No retry logic for timeouts.
- Listening on privileged ports (<1024) requires root privileges.
- Only the first question is processed in multi-question DNS queries; other questions are discarded.
- Undefined behaviour when two DNS queries from different clients use
  the same ID within the timeout window (`WAITTIME_S` and `WAITTIME_U` in `main.cpp`).

## Build options
- `make` builds the resolver binary
- `make test` builds all prerequisites and launches test.sh
- `make debug` builds binary without optimizing, with debug data and enables 
  DEBUG_PRINT macro (more information during runtime).

## Program Execution
- **Usage:**  
  `dns -s server [-p port] -f filter_file`
- Parameters (any order):
  - `-s`: DNS resolver address (IP/domain).
  - `-p port`: listening port (default: 53).
  - `-f filter_file`: file with blocked domains.

## Launch examples
```bash
./dns -s 8.8.8.8 -f blocked_domains.txt
./dns -s 8.8.8.8 -p 5000 -f blocked_domains.txt
./dns -s dns.google -p 5000 -f blocked_domains.txt
```
## Automatic testing
Command `make test` builds prerequisites and then executes testing/test.sh, 
which launches resolver and incrementally calls the first test_parse_name binary 
and then client.py. More in [Documentation](manual.pdf).

# List of submitted files
```bash
├── LICENSE
├── Makefile
├── manual.pdf
├── README.md
│
├── include
│   ├── common.h
│   ├── dns_parser.h
│   ├── errors.h
│   ├── filter.h
│   ├── resolver.h
│   ├── sig.h
│   └── udp.h
│
├── src
│   ├── common.cpp
│   ├── dns_parser.cpp
│   ├── filter.cpp
│   ├── main.cpp
│   ├── resolver.cpp
│   ├── sig.cpp
│   └── udp.cpp
│
└── testing
    ├── client.py
    ├── example_list.txt
    ├── test_parse_name.cpp
    └── test.sh
```

# Evaluation
Strohá dokumentace
Informativní README
Modulární kód
S pomocí programu se podařilo úspěšně překládat doménová jména
Podařilo úspěšně filtrovat domény
Program se úspěšně vypořádal s promíchanými odpověďmi bez filtrace
Program se úspěšně vypořádal s promíchanými odpověďmi s filtrací
Program úspěšně filtruje subdomény
Úspěšně přeložen 1 dotaz
Úspěšně přeloženy 4 dotazy
Úspěšně přeložen 1 nefiltrovaný dotaz
Úspěšně přeloženy 4 nefiltrované dotazy
Neúspěšný test bezodpovedi.query.merlin.fit.vutbr.cz
OK_bezodpovedi.query.kazi.fit.vutbr.cz
OK_bezodpovedi.query.eva.fit.vutbr.cz
OK_bezodpovedi.query.www.fit.vutbr.cz
Neúspěšný test yoyo-bezodpovedi: chyba při zpracování dotazu, který má být filtrovaný
OK_yoyo-bezodpovedi.query.kazi.fit.vutbr.cz
OK_yoyo-bezodpovedi.query.eva.fit.vutbr.cz
OK_yoyo-bezodpovedi.query.www.fit.vutbr.cz
OK_subdomeny
Dotaz úspěšně odfiltrován
Dotaz úspěšně odfiltrován
Dotaz úspěšně odfiltrován
Dobrá rychlost programu
