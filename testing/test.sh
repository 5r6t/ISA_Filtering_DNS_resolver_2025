#!/bin/bash
#  Don't forget to run `chmod +x lex_test.sh`

# * @file udp.cpp
# * @brief Handles UDP socket creation, binding, and packet send/receive operations.
# *
# * @author Jaroslav Mervart
# * @login xmervaj00
# * @date 2025-10-11

set -e

PORT=5000
RES_ADDR="8.8.8.8"

echo ">>> Test start >>>"
cd ..

# make clean && make DEBUG=1
./dns -s $RES_ADDR -f testing/example_list.txt -p $PORT &
DNS_PID=$!

# wait until UDP port 5300 is listening (timeout ~5s)
for i in {1..2500}; do
  ss -u -ln | grep -q ":$PORT\b" && break
  sleep 0.1
done

# run the Python test
echo "_______________________________________________________________________________________"
echo "____________________        CORRECT DOMAIN         ____________________________________"
echo "_______________________________________________________________________________________"

python3 testing/client.py 127.0.0.1 $PORT example.com

echo "_______________________________________________________________________________________"
echo "____________________        BLOCKED DOMAIN         ____________________________________"
echo "_______________________________________________________________________________________"

python3 testing/client.py 127.0.0.1 $PORT 101com.com # blocked domain --> Refused error

echo "_______________________________________________________________________________________"
echo "____________________      NOT A REAL DOMAIN        ____________________________________"
echo "_______________________________________________________________________________________"

python3 testing/client.py 127.0.0.1 $PORT exgdample.com

# kill dns process
kill -SIGINT $DNS_PID
echo "<<< Test done <<<"