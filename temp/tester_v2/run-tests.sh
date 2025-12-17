#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color


# Config
WEBSERV_BINARY="./webserv"
WEBSERV_CONFIG="./config/webserv.conf"
TEST_PORT=8080

# ==========================================
# CLEAN FUNCTION
# ==========================================

clean() {
    echo -e "${YELLOW}Cleaning up test artifacts...${NC}"

    # Remove test results
    if [ -d "test_results" ]; then
        rm -rf test_results
        echo -e "  ${GREEN}✓${NC} Removed test_results/"
    fi

    # Remove runtime www directory
    if [ -d "www" ]; then
        rm -rf www
        echo -e "  ${GREEN}✓${NC} Removed www/"
    fi

    # Remove webserv log
    if [ -f "webserv.log" ]; then
        rm -f webserv.log
        echo -e "  ${GREEN}✓${NC} Removed webserv.log"
    fi

    echo -e "${GREEN}✓ Clean complete${NC}"
}

# Handle command-line arguments
if [ "$1" == "clean" ]; then
    clean
    exit 0
fi

# ==========================================
# TEST SUITE DEFINITIONS
# ==========================================

# 1. Basic Static Content (GET)
TESTS_BASIC=(
    "index.html"
    "tiny.html"
    "medium.html"
    "large.html"
    "test.jpg"
)

# 2. Error Handling (GET)
TESTS_ERRORS=(
    "errors/404_not_found.html"   # Should return 404
    "errors/403_forbidden.html"   # Should return 403 (if permissions set)
    # Add more error cases here
)

# 3. Directory Listing (GET)
TESTS_DIR=(
    "uploads/"                    # Should show autoindex or 403
)

# 4. POST Tests (Stateful)
# Format: "url|data"
TESTS_POST=(
    "uploads/new_file.txt|Hello_World_Content"
    "cgi-bin/echo.sh|CGI_POST_Data"
)

# 5. DELETE Tests (Stateful)
# We will delete files that exist in fixtures
TESTS_DELETE=(
    "test.jpg"          # Delete an image
    "tiny.html"         # Delete a text file
)

# Combine all GET tests for the batch run
ALL_GET_TESTS=("${TESTS_BASIC[@]}" "${TESTS_ERRORS[@]}" "${TESTS_DIR[@]}")

# Directories
RESULTS_DIR="test_results"
NGINX_DIR="$RESULTS_DIR/nginx"
WEBSERV_DIR="$RESULTS_DIR/webserv"

# Cleanup function - runs when script exits
cleanup() {
    echo ""
    echo -e "${YELLOW}Cleaning up...${NC}"

    # Stop nginx container
    if [ -d "docker" ]; then
        cd docker
        docker-compose down 2>/dev/null
        cd ..
    fi

    # Kill webserv if it's running
    if [ -n "$WEBSERV_PID" ]; then
        kill $WEBSERV_PID 2>/dev/null
        wait $WEBSERV_PID 2>/dev/null
    fi

    # Also kill any webserv process on port 8080 (just in case)
    pkill -f "$WEBSERV_BINARY $WEBSERV_CONFIG" 2>/dev/null

    echo -e "${GREEN}✓ Cleanup complete${NC}"
}

# Register cleanup to run on script exit
trap cleanup EXIT

# Create result directories
mkdir -p "$NGINX_DIR" "$WEBSERV_DIR"

echo -e "${BLUE}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  Webserv Tester - Comparing nginx vs webserv           ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════╝${NC}"
echo ""

start_nginx() {
    echo -e "${YELLOW}Starting nginx...${NC}"
    cd docker
    docker-compose up -d
    cd ..

    # Wait for nginx to be ready
    sleep 10

    # Check if nginx is responding
    if curl -s http://localhost:8080/tiny.html > /dev/null; then
        echo -e "${GREEN}✓ nginx is ready${NC}"
        return 0
    else
        echo -e "${RED}✗ nginx failed to start${NC}"
        return 1
    fi
}

stop_nginx() {
    echo -e "${YELLOW}Stopping nginx...${NC}"
    cd docker
    docker-compose down
    cd ..
    sleep 5
}

# test_nginx() {
#     echo -e "${YELLOW}Testing nginx...${NC}"

#     # Request tiny.html and save headers + body
#     curl -D "$NGINX_DIR/tiny_headers.txt" \
#          -o "$NGINX_DIR/tiny_body.txt" \
#          -s \
#          http://localhost:8080/tiny.html

#     echo -e "${GREEN}✓ Saved nginx response${NC}"
# }


start_webserv() {
    echo -e "${YELLOW}Starting webserv...${NC}"

    # Start webserv in background, redirect output to log
	cd ..
    $WEBSERV_BINARY $WEBSERV_CONFIG > webserv.log 2>&1 &
    WEBSERV_PID=$!
	cd tester_v2

    # Just wait for it to start
    sleep 2

    # Check if process is still alive (don't send request yet)
    if kill -0 $WEBSERV_PID 2>/dev/null; then
        echo -e "${GREEN}✓ webserv started (PID: $WEBSERV_PID)${NC}"
        return 0
    else
        echo -e "${RED}✗ webserv process died immediately${NC}"
        echo -e "${YELLOW}Check webserv.log for errors${NC}"
        return 1
    fi
}

stop_webserv() {
    echo -e "${YELLOW}Stopping webserv...${NC}"

    # Kill the webserv process
    if [ -n "$WEBSERV_PID" ]; then
        kill $WEBSERV_PID 2>/dev/null
        wait $WEBSERV_PID 2>/dev/null
        echo -e "${GREEN}✓ webserv stopped${NC}"
    fi

    sleep 3
}


# test_webserv() {
#     echo -e "${YELLOW}Testing webserv...${NC}"

#     # Add a small delay and use HTTP/1.0 explicitly
#     curl -D "$WEBSERV_DIR/tiny_headers.txt" \
#          -o "$WEBSERV_DIR/tiny_body.txt" \
#          -s \
#          --http1.0 \
#          http://localhost:8080/tiny.html

#     # Wait a bit before checking
#     sleep 1

#     # Check if webserv is still alive
#     if ! kill -0 $WEBSERV_PID 2>/dev/null; then
#         echo -e "${RED}⚠ webserv died after request${NC}"
#     else
#         echo -e "${GREEN}✓ Saved webserv response${NC}"
#     fi
# }

run_batch_nginx() {
    echo -e "${YELLOW}=== Starting Nginx Batch ===${NC}"

	reset_env

    start_nginx

    # Give it a moment to be fully ready
    sleep 2

    for test_case in "${ALL_GET_TESTS[@]}"; do
        perform_request "nginx" "$test_case"
    done

    # # Run POST Tests
    # for test_case in "${TESTS_POST[@]}"; do
    #     # Split "url|data" into variables
    #     IFS='|' read -r url data <<< "$test_case"
    #     perform_request "nginx" "$url" "POST" "$data"
    # done

    # Run DELETE Tests
    # for test_case in "${TESTS_DELETE[@]}"; do
    #     perform_request "nginx" "$test_case" "DELETE"
    # done

    stop_nginx
}

run_batch_webserv() {
    echo -e "${YELLOW}=== Starting Webserv Batch ===${NC}"

	reset_env

    start_webserv

    # Give it a moment to be fully ready
    sleep 1

    for test_case in "${ALL_GET_TESTS[@]}"; do
        perform_request "webserv" "$test_case"
    done

    # Run POST Tests
    for test_case in "${TESTS_POST[@]}"; do
        # Split "url|data" into variables
        IFS='|' read -r url data <<< "$test_case"
        perform_request "webserv" "$url" "POST" "$data"
    done

    # Run DELETE Tests
    for test_case in "${TESTS_DELETE[@]}"; do
        perform_request "webserv" "$test_case" "DELETE"
    done

    stop_webserv
}

# ==========================================
# CORE FUNCTIONS
# ==========================================

# Usage: reset_env
reset_env() {
    echo -e "${BLUE}Restoring test environment...${NC}"

    # Ensure www directory exists
    mkdir -p www

    # Clear the runtime directory
    # We use find to delete contents but keep the directory itself
    rm -rf www/*

    # Copy fresh fixtures
    cp -r fixtures/* www/

    # Ensure permissions are correct for testing
    chmod -R 755 www
    chmod 777 www/uploads
}

# Usage: perform_request "server_name" "test_file" "method" "optional_data"
perform_request() {
    local server_name=$1
    local test_file=$2
    local method=${3:-GET}  # Default to GET if not provided
    local data=$4           # Optional data for POST

    local output_dir="test_results/$server_name"

    # Sanitize filename for storage
    local save_name="$test_file"
    if [[ "$test_file" == */ ]]; then
        save_name="${test_file%/}_dir_listing"
    fi
    # Replace / with _ to flatten paths (e.g., errors/404.html -> errors_404.html)
    save_name="${save_name//\//_}"

    # Add method to filename to avoid collisions (e.g. GET_tiny.html vs DELETE_tiny.html)
    save_name="${method}_${save_name}"

    # Build curl command
    local curl_cmd="curl -i -s --http1.0 -X $method"

    # Add data if present
    if [ -n "$data" ]; then
        curl_cmd="$curl_cmd -d '$data'"
    fi

    # Execute
    # We use eval because adding quotes to $curl_cmd variable is tricky in bash
    eval "$curl_cmd http://localhost:$TEST_PORT/$test_file" > "$output_dir/$save_name.raw"

    # Separate Headers and Body
    sed '/^\r$/q' "$output_dir/$save_name.raw" > "$output_dir/$save_name.headers"
    sed '1,/^\r$/d' "$output_dir/$save_name.raw" > "$output_dir/$save_name.body"

    echo -e "${BLUE}[$server_name]${NC} $method $test_file"
}

compare_all_results() {
    echo -e "${BLUE}╔════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║  Comparing All Results                                 ║${NC}"
    echo -e "${BLUE}╚════════════════════════════════════════════════════════╝${NC}"
    echo ""

    local total_passed=0
    local total_failed=0

    # Print table header
    printf "%-40s %-10s %-10s %-10s %-10s\n" "TEST CASE" "STATUS" "TYPE" "LENGTH" "BODY"
    echo "------------------------------------------------------------------------------------"

        # Helper function to compare a single test
    check_test() {
        local test_name="$1"
        local method="$2"

        # Sanitize filename (must match perform_request logic!)
        local save_name="$test_name"
        if [[ "$test_name" == */ ]]; then
            save_name="${test_name%/}_dir_listing"
        fi
        # Replace / with _ to flatten paths
        save_name="${save_name//\//_}"
        save_name="${method}_${save_name}"

        local display_name="[$method] $test_name"
        if [ ${#display_name} -gt 38 ]; then display_name="${display_name:0:35}..."; fi

        # --- SPECIAL LOGIC FOR POST ---
        if [ "$method" == "POST" ]; then
            # Check if header file exists and is not empty
            if [ ! -s "$WEBSERV_DIR/${save_name}.headers" ]; then
                printf "%-40s ${RED}%-10s${NC} No response from server\n" "$display_name" "FAIL"
                ((total_failed++))
                return
            fi

            # For POST, we only check Webserv success (200/201)
            # We skip file check because filename might be random/timestamped
            local webserv_status=$(head -1 "$WEBSERV_DIR/${save_name}.headers" 2>/dev/null | awk '{print $2}' | tr -d '\r')

            # Check 1: Status Code (Should be 200 or 201)
            if [[ "$webserv_status" != "200" && "$webserv_status" != "201" ]]; then
                printf "%-40s ${RED}%-10s${NC} Status: $webserv_status (Expected 200/201)\n" "$display_name" "FAIL"
                ((total_failed++))
            else
                 printf "%-40s ${GREEN}%-10s${NC} ${GREEN}CREATED${NC}      ${GREEN}OK${NC}\n" "$display_name" "PASS"
                 ((total_passed++))
            fi
            return
        fi

        # --- SPECIAL LOGIC FOR DELETE ---
        if [ "$method" == "DELETE" ]; then
            # Check if header file exists and is not empty
            if [ ! -s "$WEBSERV_DIR/${save_name}.headers" ]; then
                printf "%-40s ${RED}%-10s${NC} No response from server\n" "$display_name" "FAIL"
                ((total_failed++))
                return
            fi

            local webserv_status=$(head -1 "$WEBSERV_DIR/${save_name}.headers" 2>/dev/null | awk '{print $2}' | tr -d '\r')

            # Check 1: Status Code (200, 202, 204 are all valid for DELETE)
            if [[ "$webserv_status" != "200" && "$webserv_status" != "202" && "$webserv_status" != "204" ]]; then
                printf "%-40s ${RED}%-10s${NC} Status: $webserv_status (Expected 200/204)\n" "$display_name" "FAIL"
                ((total_failed++))
                return
            fi

            # Check 2: File Absence (Did the delete work?)
            if [ ! -f "www/$test_name" ]; then
                 printf "%-40s ${GREEN}%-10s${NC} ${GREEN}DELETED${NC}      ${GREEN}OK${NC}\n" "$display_name" "PASS"
                 ((total_passed++))
            else
                 printf "%-40s ${RED}%-10s${NC} File still exists in www/\n" "$display_name" "FAIL"
                 ((total_failed++))
            fi
            return
        fi
        # ------------------------------

        # --- STANDARD GET COMPARISON ---
        local failed_this_test=0

        # Check if header files exist and are not empty
        if [ ! -s "$NGINX_DIR/${save_name}.headers" ] || [ ! -s "$WEBSERV_DIR/${save_name}.headers" ]; then
            printf "%-40s ${RED}%-10s${NC} Missing or empty response\n" "$display_name" "FAIL"
            ((total_failed++))
            return
        fi

        # 1. Extract and compare Status
        local nginx_status=$(head -1 "$NGINX_DIR/${save_name}.headers" 2>/dev/null | awk '{print $2}' | tr -d '\r')
        local webserv_status=$(head -1 "$WEBSERV_DIR/${save_name}.headers" 2>/dev/null | awk '{print $2}' | tr -d '\r')
        if [ "$nginx_status" != "$webserv_status" ]; then
            failed_this_test=1
        fi

        # 2. Extract and compare Content-Length
        local nginx_len=$(grep -i "^Content-Length:" "$NGINX_DIR/${save_name}.headers" 2>/dev/null | awk '{print $2}' | tr -d '\r')
        local webserv_len=$(grep -i "^Content-Length:" "$WEBSERV_DIR/${save_name}.headers" 2>/dev/null | awk '{print $2}' | tr -d '\r')
        if [ "$nginx_len" != "$webserv_len" ]; then
            failed_this_test=1
        fi

        # 3. Compare Body
        if ! diff -q "$NGINX_DIR/${save_name}.body" "$WEBSERV_DIR/${save_name}.body" > /dev/null 2>&1; then
            failed_this_test=1
        fi

        # Print Result
        local display_name="[$method] $test_name"
        # Truncate if too long
        if [ ${#display_name} -gt 38 ]; then display_name="${display_name:0:35}..."; fi

        if [ $failed_this_test -eq 0 ]; then
            printf "%-40s ${GREEN}%-10s${NC} ${GREEN}OK${NC}         ${GREEN}OK${NC}         ${GREEN}OK${NC}\n" "$display_name" "PASS"
            ((total_passed++))
        else
            printf "%-40s ${RED}%-10s${NC} " "$display_name" "FAIL"
            if [ "$nginx_status" == "$webserv_status" ]; then echo -ne "${GREEN}OK${NC}         "; else echo -ne "${RED}DIFF${NC}       "; fi
            if [ "$nginx_len" == "$webserv_len" ]; then echo -ne "${GREEN}OK${NC}         "; else echo -ne "${RED}DIFF${NC}       "; fi
            if diff -q "$NGINX_DIR/${save_name}.body" "$WEBSERV_DIR/${save_name}.body" > /dev/null 2>&1; then echo -e "${GREEN}OK${NC}"; else echo -e "${RED}DIFF${NC}"; fi
            ((total_failed++))
        fi
    }

    # 1. Run GET Comparisons
    for test_case in "${ALL_GET_TESTS[@]}"; do
        check_test "$test_case" "GET"
    done

    # 2. Run POST Comparisons
    for test_case in "${TESTS_POST[@]}"; do
        IFS='|' read -r url data <<< "$test_case"
        check_test "$url" "POST"
    done

    # 3. Run DELETE Comparisons
    for test_case in "${TESTS_DELETE[@]}"; do
        check_test "$test_case" "DELETE"
    done

    echo ""
    echo "------------------------------------------------------------------------------------"
    echo -e "Summary: ${GREEN}$total_passed Passed${NC}, ${RED}$total_failed Failed${NC}"

    if [ $total_failed -eq 0 ]; then return 0; else return 1; fi
}

# compare_responses() {
#     echo -e "${BLUE}╔════════════════════════════════════════════════════════╗${NC}"
#     echo -e "${BLUE}║  Comparing nginx vs webserv responses                  ║${NC}"
#     echo -e "${BLUE}╚════════════════════════════════════════════════════════╝${NC}"
#     echo ""

#     local test_name="$1"  # e.g., "tiny.html"
#     local passed=0
#     local failed=0

# 	# Extract status codes
#     # Format: "HTTP/1.1 200 OK" or "HTTP/1.0 200 OK"
#     nginx_status=$(head -1 "$NGINX_DIR/${test_name}_headers.txt" | awk '{print $2}' | tr -d '\r')
# 	webserv_status=$(head -1 "$WEBSERV_DIR/${test_name}_headers.txt" | awk '{print $2}' | tr -d '\r')

#     echo -n "Status code... "
#     if [ "$nginx_status" == "$webserv_status" ]; then
#         echo -e "${GREEN}✓ MATCH${NC} ($nginx_status)"
#         ((passed++))
#     else
#         echo -e "${RED}✗ DIFFER${NC} (nginx: ${nginx_status}, webserv: ${webserv_status})"
#         ((failed++))
#     fi

# 	    # Extract Content-Type
# 	nginx_ctype=$(grep -i "^Content-Type:" "$NGINX_DIR/${test_name}_headers.txt" | cut -d' ' -f2- | tr -d '\r')
# 	webserv_ctype=$(grep -i "^Content-Type:" "$WEBSERV_DIR/${test_name}_headers.txt" | cut -d' ' -f2- | tr -d '\r')

#     echo -n "Content-Type... "
#     if [ "$nginx_ctype" == "$webserv_ctype" ]; then
#         echo -e "${GREEN}✓ MATCH${NC} ($nginx_ctype)"
#         ((passed++))
#     else
#         echo -e "${RED}✗ DIFFER${NC} (nginx: ${nginx_ctype}, webserv: ${webserv_ctype})"
#         ((failed++))
#     fi

# 	    # Extract Content-Length
# 	nginx_clen=$(grep -i "^Content-Length:" "$NGINX_DIR/${test_name}_headers.txt" | awk '{print $2}' | tr -d '\r')
# 	webserv_clen=$(grep -i "^Content-Length:" "$WEBSERV_DIR/${test_name}_headers.txt" | awk '{print $2}' | tr -d '\r')

#     echo -n "Content-Length... "
#     if [ "$nginx_clen" == "$webserv_clen" ]; then
#         echo -e "${GREEN}✓ MATCH${NC} ($nginx_clen bytes)"
#         ((passed++))
#     else
#         echo -e "${RED}✗ DIFFER${NC} (nginx: ${nginx_clen}, webserv: ${webserv_clen})"
#         ((failed++))
#     fi

#     # Compare bodies (easiest - exact match)
#     echo -n "Body content... "
#     if diff -q "$NGINX_DIR/${test_name}_body.txt" "$WEBSERV_DIR/${test_name}_body.txt" > /dev/null 2>&1; then
#         echo -e "${GREEN}✓ MATCH${NC}"
#         ((passed++))
#     else
#         echo -e "${RED}✗ DIFFER${NC}"
#         echo -e "${YELLOW}  Run: diff $NGINX_DIR/${test_name}_body.txt $WEBSERV_DIR/${test_name}_body.txt${NC}"
#         ((failed++))
#     fi

# 	    # Print summary
#     echo ""
#     echo -e "${BLUE}Results:${NC}"
#     echo -e "  ${GREEN}Passed: $passed${NC}"
#     echo -e "  ${RED}Failed: $failed${NC}"
#     echo ""

#     if [ $failed -eq 0 ]; then
#         echo -e "${GREEN}✓ All checks passed!${NC}"
#         return 0
#     else
#         echo -e "${RED}✗ Some checks failed${NC}"
#         return 1
#     fi
# }

# ============================================================================
# MAIN EXECUTION
# ============================================================================

# Clean up old results
rm -rf "$RESULTS_DIR"
mkdir -p "$NGINX_DIR" "$WEBSERV_DIR"

# 1. Run Nginx Batch
run_batch_nginx

echo ""

# 2. Run Webserv Batch
run_batch_webserv

echo ""

# 3. Compare All
compare_all_results
