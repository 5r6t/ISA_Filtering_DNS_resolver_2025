#!/bin/bash
#  Don't forget to run `chmod +x lex_test.sh`

# * @file udp.cpp
# * @brief Handles UDP socket creation, binding, and packet send/receive operations.
# *
# * @author Jaroslav Mervart
# * @login xmervaj00
# * @date 2025-10-11

set -e

PORT=5300
RES_ADDR="8.8.8.8"

echo ">>> Test start >>>"
cd ..

# make clean && make DEBUG=1
./dns -s $RES_ADDR -f testing/example_list.txt -p $PORT &
# start dns in background, log to /tmp/dns.log
./dns -s $RES_ADDR -f testing/example_list.txt -p $PORT &
DNS_PID=$!

# wait until UDP port 5300 is listening (timeout ~5s)
for i in {1..250}; do
  ss -u -ln | grep -q ":$PORT\b" && break
  sleep 0.1
done

# run the Python test
python3 testing/dns_test_client.py 127.0.0.1 $PORT example.com

cd testing/
echo "<<< Test done <<<"