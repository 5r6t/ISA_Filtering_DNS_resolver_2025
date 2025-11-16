#!/bin/bash
#  Don't forget to run `chmod +x lex_test.sh`

# * @file test.sh
# * @brief automated testing of dns resolver
# *
# * @author Jaroslav Mervart
# * @login xmervaj00
# * @date 2025-10-11
set -e

PORT=5000
RES_ADDR="8.8.8.8"

echo ">>> Test start >>>"
cd "$(dirname "$0")/.."

# Start DNS server
./dns -s "$RES_ADDR" -f testing/example_list.txt -p "$PORT" &
DNS_PID=$!

# Ensure cleanup even on error
trap "kill -SIGINT $DNS_PID 2>/dev/null" EXIT

# Wait until UDP port is listening
for i in {1..250}; do
  ss -u -ln | grep -q ":$PORT\b" && break
  sleep 0.1
done

echo "_______________________________________________________________________________________"
echo "____________________        CORRECT DOMAIN         ____________________________________"
echo "_______________________________________________________________________________________"

python3 testing/client.py 127.0.0.1 $PORT example.com

echo "_______________________________________________________________________________________"
echo "____________________        BLOCKED DOMAIN         ____________________________________"
echo "_______________________________________________________________________________________"

python3 testing/client.py 127.0.0.1 $PORT 101com.com

echo "_______________________________________________________________________________________"
echo "____________________      NOT A REAL DOMAIN        ____________________________________"
echo "_______________________________________________________________________________________"

python3 testing/client.py 127.0.0.1 $PORT exgdample.com

echo "_______________________________________________________________________________________"
echo "____________________      IPV6 TEST (LOOPBACK)     ____________________________________"
echo "_______________________________________________________________________________________"

python3 testing/client.py ::1 $PORT example.com

echo "<<< Test done <<<"