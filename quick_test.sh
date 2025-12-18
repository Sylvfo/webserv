#!/bin/bash

# Test rapide et simple

GREEN='\033[1;32m'
RED='\033[1;31m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
RESET='\033[0m'

PORT=2224
HOST="localhost"
PASSED=0
FAILED=0
SERVER_PID=""

cleanup() {
    if [ ! -z "$SERVER_PID" ]; then
        echo -e "\n${YELLOW}Arrêt du serveur...${RESET}"
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
}

trap cleanup EXIT

check() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${RESET} $2"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗${RESET} $2"
        FAILED=$((FAILED + 1))
    fi
}

echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
echo -e "${BLUE}  TEST RAPIDE RECEIVEHEADER/BODY${RESET}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}\n"

# Lancer le serveur
echo -e "${YELLOW}Lancement du serveur...${RESET}"
./webserv config/default.conf > /tmp/webserv_test.log 2>&1 &
SERVER_PID=$!
sleep 2

if ! timeout 2 bash -c "echo > /dev/tcp/${HOST}/${PORT}" 2>/dev/null; then
    echo -e "${RED}✗ Serveur non disponible${RESET}"
    exit 1
fi
echo -e "${GREEN}✓ Serveur lancé (PID: $SERVER_PID)${RESET}\n"

echo -e "${BLUE}Tests GET:${RESET}"
curl -s --max-time 3 http://${HOST}:${PORT}/ > /dev/null 2>&1
check $? "GET simple"

echo -e "\n${BLUE}Tests POST petits:${RESET}"
curl -s --max-time 3 -X POST -d "test=data" http://${HOST}:${PORT}/upload > /dev/null 2>&1
check $? "POST 10 bytes"

curl -s --max-time 3 -X POST -d "$(printf 'A%.0s' {1..100})" http://${HOST}:${PORT}/upload > /dev/null 2>&1
check $? "POST 100 bytes"

curl -s --max-time 3 -X POST -d "$(printf 'B%.0s' {1..1000})" http://${HOST}:${PORT}/upload > /dev/null 2>&1
check $? "POST 1KB"

echo -e "\n${BLUE}Tests POST gros:${RESET}"
dd if=/dev/zero of=/tmp/test_10k.dat bs=1024 count=10 2>/dev/null
curl -s --max-time 5 -X POST --data-binary @/tmp/test_10k.dat http://${HOST}:${PORT}/upload > /dev/null 2>&1
check $? "POST 10KB"

dd if=/dev/zero of=/tmp/test_100k.dat bs=1024 count=100 2>/dev/null
curl -s --max-time 5 -X POST --data-binary @/tmp/test_100k.dat http://${HOST}:${PORT}/upload > /dev/null 2>&1
check $? "POST 100KB"

dd if=/dev/zero of=/tmp/test_1m.dat bs=1024 count=1024 2>/dev/null
curl -s --max-time 10 -X POST --data-binary @/tmp/test_1m.dat http://${HOST}:${PORT}/upload > /dev/null 2>&1
check $? "POST 1MB"

echo -e "\n${BLUE}Tests headers:${RESET}"
curl -s --max-time 3 -H "X-Custom-1: value1" -H "X-Custom-2: value2" http://${HOST}:${PORT}/ > /dev/null 2>&1
check $? "Multiples headers"

echo -e "\n${BLUE}Tests edge cases:${RESET}"
curl -s --max-time 3 -X DELETE http://${HOST}:${PORT}/test.txt > /dev/null 2>&1
check $? "DELETE"

curl -s --max-time 3 -X POST -H "Content-Length: 0" http://${HOST}:${PORT}/upload > /dev/null 2>&1
check $? "POST Content-Length: 0"

echo -e "\n${BLUE}Tests multiples:${RESET}"
SUCCESS=0
for i in {1..20}; do
    curl -s --max-time 2 http://${HOST}:${PORT}/ > /dev/null 2>&1
    [ $? -eq 0 ] && SUCCESS=$((SUCCESS + 1))
done
[ $SUCCESS -eq 20 ] && check 0 "20 requêtes séquentielles ($SUCCESS/20)" || check 1 "20 requêtes séquentielles ($SUCCESS/20)"

echo -e "\n${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
echo -e "${BLUE}  RÉSUMÉ${RESET}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
TOTAL=$((PASSED + FAILED))
echo -e "\nTotal: ${TOTAL}"
echo -e "${GREEN}Réussis: ${PASSED}${RESET}"
echo -e "${RED}Échoués: ${FAILED}${RESET}"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}🎉 TOUS LES TESTS PASSENT! 🎉${RESET}\n"
    exit 0
else
    echo -e "\n${YELLOW}⚠️  ${FAILED} test(s) échoué(s)${RESET}\n"
    exit 1
fi
