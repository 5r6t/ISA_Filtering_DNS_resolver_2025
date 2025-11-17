# Basic information
- author: Jaroslav Mervart
- xlogin: xmervaj00
- date created:30th of September 2025

# Short description of the program

## Extensions

## Limitations

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

###################################################################
```bash
xmervaj00@merlin: ./dns -s 8.8.8.8 -p 5000 -f testing/example_list.txt 
resolver.cpp:75     |    resolve_host | Resolved 8.8.8.8 -> 8.8.8.8:53
udp.cpp:104         | bind_udp_socket | Socket bound to port 5000 (family IPv6)
filter.cpp:93       |     filter_load | Filter list updated
main.cpp:139        |         runtime | Listening on port 5000, upstream "8.8.8.8" socket ready
main.cpp:171        |         runtime | CLIENT QUERY RECEIVED
udp.cpp:126         |     udp_receive | Received 52 bytes via UDP.
dns_parser.cpp:114  |   read_dns_name | parsed DNS name (may be partial): example.com
dns_parser.cpp:186  |     parse_dns_q | ID: 44409
dns_parser.cpp:187  |     parse_dns_q | Flags: 288
dns_parser.cpp:188  |     parse_dns_q | QDCOUNT: 1
dns_parser.cpp:189  |     parse_dns_q | QNAME: example.com
dns_parser.cpp:190  |     parse_dns_q | QTYPE: 1
dns_parser.cpp:191  |     parse_dns_q | QCLASS:1
dns_parser.cpp:146  |      is_A_query | Query type A
filter.cpp:125      |      is_blocked |  example.com is allowed.
main.cpp:95         |   handle_client | Stored mapping id=44409
udp.cpp:155         |        udp_send | Sent 52 bytes via UDP
main.cpp:181        |         runtime | REPLY FROM UPSTREAM
udp.cpp:126         |     udp_receive | Received 136 bytes via UDP.
main.cpp:117        | handle_upstream | Forwarding reply id=44409 to client
udp.cpp:155         |        udp_send | Sent 136 bytes via UDP
```
```bash
jackie@d02-0204b: dig @147.229.176.19 -p 5000 example.com

; <<>> DiG 9.18.41 <<>> @147.229.176.19 -p 5000 example.com
; (1 server found)
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 44409
;; flags: qr rd ra ad; QUERY: 1, ANSWER: 6, AUTHORITY: 0, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 512
;; QUESTION SECTION:
;example.com.                   IN      A

;; ANSWER SECTION:
example.com.            107     IN      A       23.192.228.84
example.com.            107     IN      A       23.220.75.245
example.com.            107     IN      A       23.192.228.80
example.com.            107     IN      A       23.215.0.136
example.com.            107     IN      A       23.220.75.232
example.com.            107     IN      A       23.215.0.138

;; Query time: 5 msec
;; SERVER: 147.229.176.19#5000(147.229.176.19) (UDP)
;; WHEN: Sun Nov 16 19:56:44 CET 2025
;; MSG SIZE  rcvd: 136
```
###################################################################
# Merlin
```bash
dig -6 @2001:67c:1220:a14::1037  -p 5000 example.com
; <<>> DiG 9.20.15-1~deb13u1~bpo12+1-Debian <<>> -6 @2001:67c:1220:a14::1037 -p 5000 example.com
; (1 server found)
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 36191
;; flags: qr rd ra ad; QUERY: 1, ANSWER: 6, AUTHORITY: 0, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 512
;; QUESTION SECTION:
;example.com.                   IN      A

;; ANSWER SECTION:
example.com.            146     IN      A       23.220.75.245

;; Query time: 8 msec
;; SERVER: 2001:67c:1220:a14::1037#5000(2001:67c:1220:a14::1037) (UDP)
;; WHEN: Sun Nov 16 19:36:05 CET 2025
;; MSG SIZE  rcvd: 136
```
# Mine
```bash
sudo ./dns -s 8.8.8.8 -f testing/example_list.txt -p 5000
resolver.cpp:75     |    resolve_host | Resolved 8.8.8.8 -> 8.8.8.8:53
filter.cpp:93       |     filter_load | Filter list updated
main.cpp:139        |         runtime | Listening on port 5000, upstream "8.8.8.8" socket ready
main.cpp:171        |         runtime | CLIENT QUERY RECEIVED
udp.cpp:126         |     udp_receive | Received 52 bytes via UDP.
dns_parser.cpp:114  |   read_dns_name | parsed DNS name (may be partial): example.com
dns_parser.cpp:186  |     parse_dns_q | ID: 36191
dns_parser.cpp:187  |     parse_dns_q | Flags: 288
dns_parser.cpp:188  |     parse_dns_q | QDCOUNT: 1
dns_parser.cpp:189  |     parse_dns_q | QNAME: example.com
dns_parser.cpp:190  |     parse_dns_q | QTYPE: 1
dns_parser.cpp:191  |     parse_dns_q | QCLASS:1
dns_parser.cpp:146  |      is_A_query | Query type A
filter.cpp:125      |      is_blocked |  example.com is allowed.
main.cpp:95         |   handle_client | Stored mapping id=36191
udp.cpp:155         |        udp_send | Sent 52 bytes via UDP
main.cpp:181        |         runtime | REPLY FROM UPSTREAM
udp.cpp:126         |     udp_receive | Received 136 bytes via UDP.
main.cpp:117        | handle_upstream | Forwarding reply id=36191 to client
udp.cpp:155         |        udp_send | Sent 136 bytes via UDP
```
###################################################################

- If special library versions are required (available on merlin), note in **documentation & README**.
- Submit `.tar` archive named **xlogin00.tar** via IS VUT (not compressed further).

- Archive must include:
  - **Source code** (with author & login in header, file names per assignment).
  - **Makefile** (must build source).
  - **Documentation (manual.pdf)**:
    - intro, design, implementation, usage, testing (methods & results),
    - structured like a technical report (title page, ToC, numbered chapters, references, etc.).
  - **README**:
    - author name & login, date, program description (extensions/limits),
    - run example, list of submitted files.
  - Other required files depending on assignment.
- Missing features → clearly documented in **manual.pdf** & **README**.
- Follow Unix conventions, structured modular code, comments, formatting.

- Thorough testing required; include results in documentation.
- External code (tutorials/examples) → mark clearly, cite source & license
- **Generated code (e.g. ChatGPT)** = plagiarism.

- Before submission: verify names, files, Makefile build on target platform.

## Grading
- **Max: 20 points**
  - 15 – functional application
  - 5 – documentation (only if code works)
- Deductions:
  - messy/uncommented code: -7
  - missing/faulty Makefile: -4
  - poor/missing documentation: -5
  - wrong input/output/config format: -10
  - cannot compile/run/test: 0
  - late submission: 0
  - not following assignment: 0
  - non-functional code: 0
  - plagiarism/shared code: 0 + disciplinary action

## Testing & Validation Checklist

- Use tools (Wireshark, dig, openssl, etc.) where relevant.
- Create **repeatable automated tests** (unit, integration, system).
- Common regex mistakes:
  - unnecessary use,
  - incorrect syntax (e.g. limiting URLs wrongly).
- Verify:
  - file naming & type (.tar unpackable with `tar`),
  - buildable with `make`,
  - correct output file in project root,
  - no runtime crashes,
  - inclusion of **README & documentation**,
  - proper project root structure,
  - meaningful tests,
  - documentation shows understanding of theory, design, testing, and deviations.

# ISA Project Variant – Filtering DNS Resolver

## Task
- Write program **dns**:
  - Filter **A-type queries** targeting domains from a given blocklist (and their subdomains).
  - Forward all other queries unchanged to a specified resolver.

## Allowed Libraries
- Sockets, networking (`netinet/*`, `sys/*`, `arpa/*`).
- Threads (`pthread`), signals, time.
- Standard **C (ISO/ANSI, POSIX)** and **C++ STL**.
- **No other libraries allowed.**



## Supported Queries
- Only **A-type queries**.
- Transport: **UDP**.
- No **DNSSEC** support required.

## Output
- By default: **no output**.
- Optional: `-v` → verbose logging in custom format.

## Filter File Format
- Text (ASCII), one domain per line.
- Ignore:
  - empty lines,
  - lines starting with `#`.
- Must handle line endings from Linux, Windows, macOS.

## Additional Notes
- Read common ISA project rules before submission.
- Document all extensions in **README & manual.pdf** (no extra points).
- Program must handle **invalid input gracefully**.
- Errors → print to **stderr** clearly.
- If spec missing details → document chosen solution & rationale.

## Reference Environment
- Program must compile & run on:
  - **eva.fit.vutbr.cz**
  - **merlin.fit.vutbr.cz**
- Should be portable (Linux, FreeBSD, other architectures/distros/library versions).
- If requiring minimum library version (available on eva/merlin), state in **documentation & README**.


# GOOD TO KNOW
> ad více dotazů v jedné zprávě DNS: takové zprávy se v praxi nevyskytují, protože není jasná sémantika některých polí v očekávané odpovědi (např. AA, RCODE), vizte atké např. https://stackoverflow.com/questions/4082081/requesting-a-and-aaaa-records-in-single-dns-query/4083071#4083071. Zasílat více dotazů v jedné zprávě při testování nebudu, nicméně v dokumentaci doporučuji vysvětlit, jak se program v takové situaci zachová.
> Pouzivame UDP -- max velkost packetu 512bytes a iba jeden
> DNS header = 12 bytes


# SOURCES
- dual stack https://stackoverflow.com/questions/22075363/dual-stack-with-one-socket
- ipv6, sockaddr_storage https://datatracker.ietf.org/doc/html/rfc2553
- DNS https://mislove.org/teaching/cs4700/spring11/handouts/project1-primer.pdf
- DNS https://learn.microsoft.com/en-us/windows-server/networking/dns/message-formats
- [RFC 1035 – Domain Names](https://datatracker.ietf.org/doc/html/rfc1035)