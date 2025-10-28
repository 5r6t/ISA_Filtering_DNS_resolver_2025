#!/usr/bin/env python3

# WARNING USE AT YOUR OWN DISCRETION == AI GENERATED !!! 

# Minimal DNS UDP client (no external libs). Usage:
# python3 dns_test_client.py <host> <port> <domain>

import sys, socket, struct, random

RESP_TIMEOUT = 5000.0


def build_query(domain: str) -> bytes:
    # Header: ID, Flags, QDCOUNT, ANCOUNT, NSCOUNT, ARCOUNT
    tid = random.getrandbits(16)
    flags = 0x0100            # standard recursive query
    qdcount = 1
    header = struct.pack("!HHHHHH", tid, flags, qdcount, 0, 0, 0)

    # QNAME: split labels, length-prefixed, terminated with 0
    parts = domain.strip(".").split(".")
    qname = b"".join(struct.pack("B", len(p)) + p.encode() for p in parts) + b"\x00"

    qtype = 1   # A
    qclass = 1  # IN
    question = qname + struct.pack("!HH", qtype, qclass)
    return header + question, tid

def hexdump(b: bytes) -> str:
    return " ".join(f"{x:02x}" for x in b)

def main():
    if len(sys.argv) != 4:
        print("Usage: dns_test_client.py <host> <port> <domain>")
        sys.exit(1)
    host, port_s, domain = sys.argv[1], int(sys.argv[2]), sys.argv[3]

    packet, tid = build_query(domain)
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(RESP_TIMEOUT)
    try:
        s.sendto(packet, (host, port_s))
        data, addr = s.recvfrom(4096)
    except socket.timeout:
        print("CLIENT_SIDE No reply (timeout).")
        sys.exit(2)
    finally:
        s.close()

    # Quick sanity: check transaction id matches
    if len(data) >= 2:
        resp_tid = struct.unpack("!H", data[:2])[0]
        print(f"CLIENT_SIDE TXID sent={tid:#06x} reply={resp_tid:#06x}")
    print(f"CLIENT_SIDE Reply from {addr}: {len(data)} bytes")
    print(hexdump(data))

if __name__ == "__main__":
    main()