#!/bin/bash

# Name of the C program executable
C_PROGRAM="./lottery"

# Output file
OUTPUT_FILE="out.txt"

# Check if the C program executable exists
if [ ! -f "$C_PROGRAM" ]; then
    echo "Error: C program executable '$C_PROGRAM' not found."
    exit 1
fi

# Check if the cases file exists
CASES_FILE="cases.txt"
if [ ! -f "$CASES_FILE" ]; then
    echo "Error: Cases file '$CASES_FILE' not found."
    exit 1
fi

# Clear the output file if it exists
> "$OUTPUT_FILE"

# Function to resolve error code to error name
resolve_error() {
    local err_code=$1
    local err_name
    err_name=$(errno "$err_code" 2>/dev/null)
    if [ $? -eq 0 ]; then
        echo "$err_name"
    else
        echo "Unknown error code: $err_code"
    fi
}

# Read the cases file line by line
while IFS= read -r IN; do
    args="$(cut -d';' -f1 <<<$IN)"
    expected_return="$(cut -d';' -f2 <<<$IN)"
    # echo "args $args expected_return $expected_return"

    echo "Running $C_PROGRAM with arguments: $args" | tee -a "$OUTPUT_FILE"
    $C_PROGRAM $args >> "$OUTPUT_FILE" 2>&1
    exit_code=$?

    # Check if an expected return value was provided
    if [[ -n "$expected_return" ]]; then
        if [ "$exit_code" -eq "$expected_return" ]; then
            echo "Success: Return value matches expected ($exit_code)" | tee -a "$OUTPUT_FILE"
        else
            echo "Error: Return value does not match expected" | tee -a "$OUTPUT_FILE"
            echo "Expected: $expected_return, Actual: $exit_code" | tee -a "$OUTPUT_FILE"
        fi
    fi

    # Resolve the error code to its name if the program failed
    if [ $exit_code -ne 0 ]; then
        err_name=$(resolve_error "$exit_code")
        echo "Exit code: $exit_code ($err_name)" | tee -a "$OUTPUT_FILE"
    fi

    echo "----------------------------------------" | tee -a "$OUTPUT_FILE"
done < "$CASES_FILE"

echo "All cases have been processed. Output saved to $OUTPUT_FILE."