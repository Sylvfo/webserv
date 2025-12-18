#!/bin/bash

# Script de test complet - Lance serveur, teste tout, arrête serveur

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
        echo -e "\n${YELLOW}Arrêt du serveur (PID: $SERVER_PID)...${RESET}"
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    rm -f /tmp/test_*.txt /tmp/large_*.dat
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

echo -e "${BLUE}"
echo "╔════════════════════════════════════════════════════════════╗"
echo "║           TEST COMPLET RECEIVEHEADER/BODY                 ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo -e "${RESET}\n"

# Lancer le serveur
echo -e "${YELLOW}1. Lancement du serveur...${RESET}"
./webserv config/default.conf > /tmp/webserv_output.log 2>&1 &
SERVER_PID=$!
sleep 2

# Vérifier que le serveur est lancé
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo -e "${RED}✗ Le serveur n'a pas démarré${RESET}"
    cat /tmp/webserv_output.log
    exit 1
fi

if ! timeout 2 bash -c "echo > /dev/tcp/${HOST}/${PORT}" 2>/dev/null; then
    echo -e "${RED}✗ Le serveur ne répond pas sur ${HOST}:${PORT}${RESET}"
    exit 1
fi

echo -e "${GREEN}✓ Serveur lancé (PID: $SERVER_PID)${RESET}\n"

# TESTS
echo -e "${BLUE}═══════════════════════════════════════════════════════════${RESET}"
echo -e "${BLUE}2. Tests basiques${RESET}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${RESET}"

# Test 1: GET simple
curl -s --max-time 5 http://${HOST}:${PORT}/ > /tmp/test_get.txt 2>&1
check $? "GET simple"

# Test 2: POST petit body
curl -s --max-time 5 -X POST -d "name=test&value=123" http://${HOST}:${PORT}/upload > /tmp/test_post_small.txt 2>&1
check $? "POST petit body (20 bytes)"

# Test 3: POST body moyen
DATA_100=$(printf 'A%.0s' {1..100})
curl -s --max-time 5 -X POST -d "$DATA_100" http://${HOST}:${PORT}/upload > /tmp/test_post_medium.txt 2>&1
check $? "POST body moyen (100 bytes)"

# Test 4: POST body 1KB
DATA_1K=$(printf 'B%.0s' {1..1024})
curl -s --max-time 5 -X POST -d "$DATA_1K" http://${HOST}:${PORT}/upload > /tmp/test_post_1k.txt 2>&1
check $? "POST body 1KB"

# Test 5: POST body 10KB
DATA_10K=$(printf 'C%.0s' {1..10240})
curl -s --max-time 5 -X POST -d "$DATA_10K" http://${HOST}:${PORT}/upload > /tmp/test_post_10k.txt 2>&1
check $? "POST body 10KB"

# Test 6: POST body 100KB
echo -e "${YELLOW}Création fichier 100KB...${RESET}"
dd if=/dev/zero of=/tmp/large_100k.dat bs=1024 count=100 2>/dev/null
curl -s --max-time 10 -X POST --data-binary @/tmp/large_100k.dat http://${HOST}:${PORT}/upload > /tmp/test_post_100k.txt 2>&1
check $? "POST body 100KB"

# Test 7: POST body 1MB
echo -e "${YELLOW}Création fichier 1MB...${RESET}"
dd if=/dev/zero of=/tmp/large_1m.dat bs=1024 count=1024 2>/dev/null
curl -s --max-time 15 -X POST --data-binary @/tmp/large_1m.dat http://${HOST}:${PORT}/upload > /tmp/test_post_1m.txt 2>&1
check $? "POST body 1MB"

echo -e "\n${BLUE}═══════════════════════════════════════════════════════════${RESET}"
echo -e "${BLUE}3. Tests headers${RESET}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${RESET}"

# Test 8: Beaucoup de headers
curl -s -H "X-Custom-1: value1" -H "X-Custom-2: value2" -H "X-Custom-3: value3" \
     -H "X-Custom-4: value4" -H "X-Custom-5: value5" -H "X-Custom-6: value6" \
     -H "X-Custom-7: value7" -H "X-Custom-8: value8" -H "X-Custom-9: value9" \
     -H "X-Custom-10: value10" http://${HOST}:${PORT}/ > /tmp/test_headers.txt 2>&1
check $? "Multiples headers"

# Test 9: Header avec valeur longue
LONG_VALUE=$(printf 'X%.0s' {1..500})
curl -s -H "X-Long-Header: $LONG_VALUE" http://${HOST}:${PORT}/ > /tmp/test_long_header.txt 2>&1
check $? "Header avec valeur longue (500 chars)"

# Test 10: Header avec valeur très longue
VERY_LONG_VALUE=$(printf 'Y%.0s' {1..2000})
curl -s -H "X-Very-Long-Header: $VERY_LONG_VALUE" http://${HOST}:${PORT}/ > /tmp/test_very_long_header.txt 2>&1
check $? "Header avec valeur très longue (2000 chars)"

echo -e "\n${BLUE}═══════════════════════════════════════════════════════════${RESET}"
echo -e "${BLUE}4. Tests edge cases${RESET}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${RESET}"

# Test 11: POST Content-Length 0
curl -s -X POST -H "Content-Length: 0" http://${HOST}:${PORT}/upload > /tmp/test_empty_body.txt 2>&1
check $? "POST Content-Length: 0"

# Test 12: DELETE
curl -s -X DELETE http://${HOST}:${PORT}/test.txt > /tmp/test_delete.txt 2>&1
check $? "DELETE request"

# Test 13: PUT
curl -s -X PUT -d "data" http://${HOST}:${PORT}/test.txt > /tmp/test_put.txt 2>&1
check $? "PUT request"

# Test 14: Requêtes multiples séquentielles
SUCCESS_COUNT=0
for i in {1..20}; do
    curl -s http://${HOST}:${PORT}/ > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    fi
done
if [ $SUCCESS_COUNT -eq 20 ]; then
    check 0 "20 requêtes séquentielles"
else
    check 1 "20 requêtes séquentielles (${SUCCESS_COUNT}/20)"
fi

# Test 15: Requêtes simultanées
echo -e "${YELLOW}Lancement 10 requêtes simultanées...${RESET}"
for i in {1..10}; do
    curl -s http://${HOST}:${PORT}/test > /tmp/test_concurrent_${i}.txt 2>&1 &
done
wait
SUCCESS_COUNT=0
for i in {1..10}; do
    if [ -f /tmp/test_concurrent_${i}.txt ]; then
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    fi
done
if [ $SUCCESS_COUNT -eq 10 ]; then
    check 0 "10 requêtes simultanées"
else
    check 1 "10 requêtes simultanées (${SUCCESS_COUNT}/10)"
fi

echo -e "\n${BLUE}═══════════════════════════════════════════════════════════${RESET}"
echo -e "${BLUE}5. Tests body fragmenté (avec netcat)${RESET}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${RESET}"

# Test 16: Body envoyé lentement
{
    echo -ne "POST /upload HTTP/1.1\r\nHost: ${HOST}\r\nContent-Length: 20\r\n\r\n"
    sleep 0.1
    echo -ne "12345"
    sleep 0.1
    echo -ne "67890"
    sleep 0.1
    echo -ne "ABCDEFGHIJ"
} | nc ${HOST} ${PORT} > /tmp/test_slow_body.txt 2>&1 &
sleep 1
if grep -q "HTTP" /tmp/test_slow_body.txt; then
    check 0 "Body envoyé lentement (fragmenté)"
else
    check 1 "Body envoyé lentement (fragmenté)"
fi

# Test 17: Header et body séparés
{
    echo -ne "POST /upload HTTP/1.1\r\nHost: ${HOST}\r\nContent-Length: 10\r\n\r\n"
    sleep 0.2
    echo -ne "0123456789"
} | nc ${HOST} ${PORT} > /tmp/test_separated.txt 2>&1 &
sleep 1
if grep -q "HTTP" /tmp/test_separated.txt; then
    check 0 "Header et body séparés"
else
    check 1 "Header et body séparés"
fi

# Test 18: Header fragmenté
{
    echo -ne "GET /index.html HTTP/1.1\r\n"
    sleep 0.1
    echo -ne "Host: ${HOST}\r\n"
    sleep 0.1
    echo -ne "User-Agent: TestClient\r\n"
    sleep 0.1
    echo -ne "\r\n"
} | nc ${HOST} ${PORT} > /tmp/test_fragmented_header.txt 2>&1 &
sleep 1
if grep -q "HTTP" /tmp/test_fragmented_header.txt; then
    check 0 "Header fragmenté"
else
    check 1 "Header fragmenté"
fi

echo -e "\n${BLUE}═══════════════════════════════════════════════════════════${RESET}"
echo -e "${BLUE}6. Tests body avec caractères spéciaux${RESET}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${RESET}"

# Test 19: Body avec newlines
curl -s -X POST -d "line1
line2
line3" http://${HOST}:${PORT}/upload > /tmp/test_newlines.txt 2>&1
check $? "Body avec newlines"

# Test 20: Body avec caractères UTF-8
curl -s -X POST -d "Héllo Wörld! 你好 🚀" http://${HOST}:${PORT}/upload > /tmp/test_utf8.txt 2>&1
check $? "Body avec UTF-8"

# Test 21: Body JSON
curl -s -X POST -H "Content-Type: application/json" \
     -d '{"name":"test","value":123,"array":[1,2,3]}' \
     http://${HOST}:${PORT}/upload > /tmp/test_json.txt 2>&1
check $? "Body JSON"

echo -e "\n${BLUE}═══════════════════════════════════════════════════════════${RESET}"
echo -e "${BLUE}7. Tests de performance${RESET}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${RESET}"

# Test 22: Upload de fichier texte
echo "Creating 500KB text file..."
dd if=/dev/urandom of=/tmp/large_500k.dat bs=1024 count=500 2>/dev/null
START_TIME=$(date +%s.%N)
curl -s -X POST --data-binary @/tmp/large_500k.dat http://${HOST}:${PORT}/upload > /tmp/test_500k.txt 2>&1
END_TIME=$(date +%s.%N)
DURATION=$(echo "$END_TIME - $START_TIME" | bc)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${RESET} Upload 500KB (${DURATION}s)"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗${RESET} Upload 500KB"
    FAILED=$((FAILED + 1))
fi

# Test 23: 50 requêtes rapides
echo -e "${YELLOW}50 requêtes GET rapides...${RESET}"
START_TIME=$(date +%s.%N)
SUCCESS_COUNT=0
for i in {1..50}; do
    curl -s http://${HOST}:${PORT}/ > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    fi
done
END_TIME=$(date +%s.%N)
DURATION=$(echo "$END_TIME - $START_TIME" | bc)
if [ $SUCCESS_COUNT -eq 50 ]; then
    echo -e "${GREEN}✓${RESET} 50 requêtes GET (${DURATION}s, ${SUCCESS_COUNT}/50)"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗${RESET} 50 requêtes GET (${DURATION}s, ${SUCCESS_COUNT}/50)"
    FAILED=$((FAILED + 1))
fi

echo -e "\n${BLUE}═══════════════════════════════════════════════════════════${RESET}"
echo -e "${BLUE}8. Vérification des logs serveur${RESET}"
echo -e "${BLUE}═══════════════════════════════════════════════════════════${RESET}"

# Vérifier les logs
if grep -q "BODY\] Complete" /tmp/webserv_output.log; then
    check 0 "Body complets détectés dans les logs"
else
    check 1 "Body complets détectés dans les logs"
fi

if grep -q "RECEIVE_HEADER\] seperator found" /tmp/webserv_output.log; then
    check 0 "Headers détectés dans les logs"
else
    check 1 "Headers détectés dans les logs"
fi

# Compter les erreurs
ERROR_COUNT=$(grep -c "\[ERROR\]" /tmp/webserv_output.log 2>/dev/null || echo "0")
echo -e "${YELLOW}Nombre d'erreurs dans les logs: ${ERROR_COUNT}${RESET}"

# RÉSUMÉ
echo -e "\n${BLUE}╔════════════════════════════════════════════════════════════╗${RESET}"
echo -e "${BLUE}║                      RÉSUMÉ FINAL                          ║${RESET}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════════╝${RESET}"
TOTAL=$((PASSED + FAILED))
echo -e "\nTotal tests: ${TOTAL}"
echo -e "${GREEN}Réussis: ${PASSED}${RESET}"
echo -e "${RED}Échoués: ${FAILED}${RESET}"

PERCENT=$((PASSED * 100 / TOTAL))
echo -e "\nTaux de réussite: ${PERCENT}%"

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}╔════════════════════════════════════════════════════════════╗${RESET}"
    echo -e "${GREEN}║              🎉 TOUS LES TESTS PASSENT! 🎉                ║${RESET}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${RESET}\n"
    exit 0
else
    echo -e "\n${YELLOW}╔════════════════════════════════════════════════════════════╗${RESET}"
    echo -e "${YELLOW}║              ⚠️  CERTAINS TESTS ONT ÉCHOUÉ               ║${RESET}"
    echo -e "${YELLOW}╚════════════════════════════════════════════════════════════╝${RESET}"
    echo -e "\n${YELLOW}Vérifiez les logs: /tmp/webserv_output.log${RESET}\n"
    exit 1
fi
