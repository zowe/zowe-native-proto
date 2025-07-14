#!/bin/bash
RESULTS_FILE="test_results_$(date +%Y%m%d_%H%M%S).txt"

{
    echo "=== Test Results - $(date) ==="
    echo

    # Dataset tests
    echo "Converting dataset tests..."
    iconv -f IBM-1047 -t UTF-8 test_zds.py > test_zds_utf8.py
    chtag -t -c UTF-8 test_zds_utf8.py

    echo "Running dataset tests..."
    export PYTHONIOENCODING=utf-8
    pytest test_zds_utf8.py -v

    echo

    # Job tests  
    echo "Converting job tests..."
    iconv -f IBM-1047 -t UTF-8 test_zjb.py > test_zjb_utf8.py
    chtag -t -c UTF-8 test_zjb_utf8.py

    echo "Running job tests..."
    pytest test_zjb_utf8.py -v

    echo

    # USS tests
    echo "Converting USS tests..."
    iconv -f IBM-1047 -t UTF-8 test_zusf.py > test_zusf_utf8.py
    chtag -t -c UTF-8 test_zusf_utf8.py

    echo "Running USS tests..."
    pytest test_zusf_utf8.py -v

    echo "Done!"
    echo "=== Test Completed - $(date) ==="
} 2>&1 | tee "$RESULTS_FILE"

echo
echo "Results saved to: $RESULTS_FILE"