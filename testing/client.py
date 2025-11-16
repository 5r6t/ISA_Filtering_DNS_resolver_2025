#!/usr/bin/env python3

# WARNING USE AT YOUR OWN DISCRETION == AI GENERATED !!! 

# Minimal DNS UDP client (no external libs). Usage:
# python3 dns_test_client.py <host> <port> <domain>

import sys, socket, struct, random

RESP_TIMEOUT = 5.0  # seconds

def build_query(domain: str):
    tid = random.getrandbits(16)
    flags = 0x0100
    header = struct.pack("!HHHHHH", tid, flags, 1, 0, 0, 0)

    qname = b''.join(
        struct.pack("B", len(p)) + p.encode()
        for p in domain.strip('.').split('.')
    ) + b'\x00'

    question = qname + struct.pack("!HH", 1, 1)
    return header + question, tid

def hexdump(b: bytes):
    return " ".join(f"{x:02x}" for x in b)

def main():
    if len(sys.argv) != 4:
        print("Usage: client.py <host> <port> <domain>")
        sys.exit(1)

    host = sys.argv[1]
    port = int(sys.argv[2])
    domain = sys.argv[3]

    packet, tid = build_query(domain)

    # Resolve host with either IPv4 or IPv6
    infos = socket.getaddrinfo(host, port, socket.AF_UNSPEC, socket.SOCK_DGRAM)
    if not infos:
        print("CLIENT_SIDE: getaddrinfo failed.")
        sys.exit(1)

    # Use the first valid address
    family, socktype, proto, _, sockaddr = infos[0]

    s = socket.socket(family, socktype, proto)
    s.settimeout(RESP_TIMEOUT)

    try:
        s.sendto(packet, sockaddr)
        data, addr = s.recvfrom(4096)
    except socket.timeout:
        print("CLIENT_SIDE: No reply (timeout)")
        sys.exit(2)
    finally:
        s.close()

    # Print reply
    resp_tid = struct.unpack("!H", data[:2])[0]
    print(f"CLIENT_SIDE: TXID sent={tid:#06x} reply={resp_tid:#06x}")
    print(f"CLIENT_SIDE: Reply from {addr}: {len(data)} bytes")
    print(hexdump(data))

if __name__ == "__main__":
    main()
