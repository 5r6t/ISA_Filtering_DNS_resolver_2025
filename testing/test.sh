#!/bin/bash
#  Don't forget to run `chmod +x test.sh`

# * @file test.sh
# * @brief automated testing of dns resolver
# *
# * @author Jaroslav Mervart
# * @login xmervaj00
# * @date 2025-10-11
#!/bin/bash

set -e

PORT=5000
RES_ADDR="8.8.8.8"
GREEN="\033[32m"; RED="\033[31m"; YELLOW="\033[33m"; RESET="\033[0m"
PASS_MSG="${GREEN}>>> PASS${RESET}"
FAIL_MSG="${RED}>>> FAIL${RESET}"

echo -e "${YELLOW}>>> Test start ${RESET}>>>"
cd "$(dirname "$0")/.."

./dns -s "$RES_ADDR" -f testing/example_list.txt -p "$PORT" &
DNS_PID=$!

# Kill dns resolver process at the end of the script
trap "kill -SIGINT $DNS_PID 2>/dev/null" EXIT

# Wait for UDP port to open
for i in {1..250}; do
  ss -u -ln | grep -q ":$PORT\b" && break
  sleep 0.1
done

# helper: run client and extract RCODE
run_test() {
    output=$(python3 testing/client.py "$1" "$PORT" "$2" || true)
    echo "$output"
    rcode=$(echo "$output" | grep "RCODE=" | sed 's/.*RCODE=//')
}

echo "_______________________________________________________________________________________"
echo "____________________        DNS NAME PARSING __________________________________________"
echo "_______________________________________________________________________________________"

./testing/test_dns_name

echo "_______________________________________________________________________________________"
echo "____________________        CORRECT DOMAIN (IPv4) _____________________________________"
echo "_______________________________________________________________________________________"

expected=0;
run_test 127.0.0.1 example.com
[ "$rcode" = "0" ] || { echo -e "${FAIL_MSG}: expected RCODE=${expected}"; exit 1; }
echo -e "${PASS_MSG}"

echo "_______________________________________________________________________________________"
echo "____________________        BLOCKED DOMAIN (IPv4) _____________________________________"
echo "_______________________________________________________________________________________"

expected=5;
run_test 127.0.0.1 101com.com
[ "$rcode" = "$expected" ] || { echo -e "${FAIL_MSG}: expected RCODE=${expected}"; exit 1; }
echo -e "${PASS_MSG}"

echo "_______________________________________________________________________________________"
echo "____________________      NOT A REAL DOMAIN (IPv4) ____________________________________"
echo "_______________________________________________________________________________________"

expected=3
run_test 127.0.0.1 exgdample.com
[ "$rcode" = "$expected" ] || { echo -e "${FAIL_MSG}: expected RCODE=${expected}"; exit 1; }
echo -e "${PASS_MSG}"

echo "_______________________________________________________________________________________"
echo "____________________      IPV6 TEST (ALLOWED) _________________________________________"
echo "_______________________________________________________________________________________"

expected=0;
run_test ::1 example.com
[ "$rcode" = "$expected" ] || { echo -e "${FAIL_MSG}: IPv6 allowed expected RCODE=${expected}"; exit 1; }
echo -e "${PASS_MSG}"

echo "_______________________________________________________________________________________"
echo "____________________      IPV6 BLOCKED DOMAIN _________________________________________"
echo "_______________________________________________________________________________________"

expected=5;
run_test ::1 101com.com
[ "$rcode" = "$expected" ] || { echo -e "${FAIL_MSG}: IPv6 blocked expected RCODE=${expected}"; exit 1; }
echo -e "${PASS_MSG}"

echo "_______________________________________________________________________________________"
echo "____________________      IPV6 NXDOMAIN _______________________________________________"
echo "_______________________________________________________________________________________"

expected=3;
run_test ::1 exgdample.com
[ "$rcode" = "$expected" ] || { echo -e "${FAIL_MSG}: IPv6 nxdomain expected RCODE=${expected}"; exit 1; }
echo -e "${PASS_MSG}"

echo "_______________________________________________________________________________________"
echo "____________________      MALFORMED QUERY _____________________________________________"
echo "_______________________________________________________________________________________"

# send 1 byte via UDP - forced malformed
malformed_output=$(python3 testing/client.py 127.0.0.1 $PORT malformed || true)
echo "$malformed_output"
# Extract RCODE from client output
expected=1;
rcode=$(echo "$malformed_output" | grep "RCODE=" | sed 's/.*RCODE=//')
[ "$rcode" = "$expected" ] || { echo -e "${FAIL_MSG}: malformed expected RCODE=${expected}"; exit 1; }
echo -e "${PASS_MSG}"

echo "_______________________________________________________________________________________"
echo "____________________      IPV6 MALFORMED QUERY ________________________________________"
echo "_______________________________________________________________________________________"

# send 1 byte via UDP - forced malformed
malformed_output=$(python3 testing/client.py 127.0.0.1 $PORT malformed || true)
echo "$malformed_output"
# Extract RCODE from client output
expected=1;
rcode=$(echo "$malformed_output" | grep "RCODE=" | sed 's/.*RCODE=//')
[ "$rcode" = "$expected" ] || { echo -e "${FAIL_MSG}: malformed expected RCODE=${expected}"; exit 1; }
echo -e "${PASS_MSG}"

############################################
echo -e "${YELLOW}>>> Test done ${RESET}>>>"