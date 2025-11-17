#!/usr/bin/env python3

# DNS UDP client
# Usage:
#   python3 dns_test_client.py <host> <port> <domain>

import sys, socket, struct, random

RESP_TIMEOUT = 5.0  # in seconds

# Errors
BAD_ARGS = 1
RES_FAIL = 2
TIMEOUT  = 3
INTERNAL = 99


def build_query(domain: str):
    tid = random.getrandbits(16)
    flags = 0x0100  # standard recursive query
    header = struct.pack("!HHHHHH", tid, flags, 1, 0, 0, 0)

    # Encode QNAME
    labels = domain.strip(".").split(".")
    qname = b"".join(struct.pack("B", len(l)) + l.encode() for l in labels) + b"\x00"

    question = qname + struct.pack("!HH", 1, 1)  # QTYPE=A QCLASS=IN
    return header + question, tid


def hexdump(b: bytes):
    return " ".join(f"{x:02x}" for x in b)


def main():
    if len(sys.argv) != 4:
        print("Usage: client.py <host> <port> <domain>")
        sys.exit(BAD_ARGS)

    host = sys.argv[1]
    port = int(sys.argv[2])
    domain = sys.argv[3]
    
    ### MALFORMED QUERY
    if domain == "malformed":
        try:
            infos = socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_DGRAM)
        except socket.gaierror:
            print("CLIENT_SIDE: getaddrinfo failed.")
            sys.exit(RES_FAIL)

        if not infos:
            print("CLIENT_SIDE: No usable address.")
            sys.exit(RES_FAIL)

        family, socktype, proto, _, sockaddr = infos[0]

        s = socket.socket(family, socktype, proto)
        s.settimeout(RESP_TIMEOUT)

        try:
            s.sendto(b"\x00", sockaddr)   # <<< MALFORMED DNS PACKET
            data, addr = s.recvfrom(4096)
        except socket.timeout:
            print("CLIENT_SIDE: No reply (timeout)")
            sys.exit(TIMEOUT)
        finally:
            s.close()

        # if server replies with <4 bytes it's malformed too
        if len(data) < 4:
            print("CLIENT_SIDE: Malformed reply (too short)")
            sys.exit(INTERNAL)

        flags = struct.unpack("!H", data[2:4])[0]
        rcode = flags & 0x000F

        print(f"CLIENT_SIDE: RCODE={rcode}")
        print(f"CLIENT_SIDE: Reply from {addr}: {len(data)} bytes")
        print(hexdump(data))
        sys.exit(rcode)

    ### NORMAL QUERY
    packet, tid = build_query(domain)

    # Resolve host (supports IPv4 + IPv6)
    try:
        infos = socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_DGRAM)
    except socket.gaierror:
        print("CLIENT_SIDE: getaddrinfo failed.")
        sys.exit(RES_FAIL)

    if not infos:
        print("CLIENT_SIDE: No usable address.")
        sys.exit(RES_FAIL)

    family, socktype, proto, _, sockaddr = infos[0]

    s = socket.socket(family, socktype, proto)
    s.settimeout(RESP_TIMEOUT)

    try:
        s.sendto(packet, sockaddr)
        data, addr = s.recvfrom(4096)
    except socket.timeout:
        print("CLIENT_SIDE: No reply (timeout)")
        sys.exit(TIMEOUT)
    finally:
        s.close()

    # Safety check: DNS header is at least 4 bytes
    if len(data) < 4:
        print("CLIENT_SIDE: Malformed reply (too short)")
        sys.exit(INTERNAL)

    # Extract fields
    resp_tid = struct.unpack("!H", data[:2])[0]
    flags = struct.unpack("!H", data[2:4])[0]
    rcode = flags & 0x000F

    print(f"CLIENT_SIDE: RCODE={rcode}")
    print(f"CLIENT_SIDE: TXID sent={tid:#06x} reply={resp_tid:#06x}")
    print(f"CLIENT_SIDE: Reply from {addr}: {len(data)} bytes")
    print(hexdump(data))

    # Exit code = RCODE
    sys.exit(rcode)


if __name__ == "__main__":
    main()
