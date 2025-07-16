RESULTS_FILE := test_results.txt

# Test files
TEST_ZDS := test_zds.py
TEST_ZJB := test_zjb.py
TEST_ZUSF := test_zusf.py

# Converted UTF-8 files
TEST_ZDS_UTF8 := test_zds_utf8.py
TEST_ZJB_UTF8 := test_zjb_utf8.py
TEST_ZUSF_UTF8 := test_zusf_utf8.py

# Default target

.PHONY: all
all: run-tests

# Run tests without saving to file
.PHONY: run-tests
run-tests: $(TEST_ZDS_UTF8) $(TEST_ZJB_UTF8) $(TEST_ZUSF_UTF8)
	@echo "=== Test Results - $(date) ==="
	@echo
	@echo "Running dataset tests..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZDS_UTF8) -v
	@echo
	@echo "Running job tests..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZJB_UTF8) -v
	@echo
	@echo "Running USS tests..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZUSF_UTF8) -v
	@echo
	@echo "All tests completed!"

# Main test target
.PHONY: test
test: $(TEST_ZDS_UTF8) $(TEST_ZJB_UTF8) $(TEST_ZUSF_UTF8)
	@echo "=== Test Results - $$(date) ===" | tee $(RESULTS_FILE)
	@echo | tee -a $(RESULTS_FILE)
	@echo "Running dataset tests..." | tee -a $(RESULTS_FILE)
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZDS_UTF8) -v 2>&1 | tee -a $(RESULTS_FILE)
	@echo | tee -a $(RESULTS_FILE)
	@echo "Running job tests..." | tee -a $(RESULTS_FILE)
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZJB_UTF8) -v 2>&1 | tee -a $(RESULTS_FILE)
	@echo | tee -a $(RESULTS_FILE)
	@echo "Running USS tests..." | tee -a $(RESULTS_FILE)
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZUSF_UTF8) -v 2>&1 | tee -a $(RESULTS_FILE)
	@echo | tee -a $(RESULTS_FILE)
	@echo "Done!" | tee -a $(RESULTS_FILE)
	@echo "=== Test Completed - $$(date) ===" | tee -a $(RESULTS_FILE)
	@echo
	@echo "Results saved to: $(RESULTS_FILE)"

# Convert individual test files from EBCDIC to UTF-8
$(TEST_ZDS_UTF8): $(TEST_ZDS)
	@echo "Converting dataset tests..."
	iconv -f IBM-1047 -t UTF-8 $(TEST_ZDS) > $(TEST_ZDS_UTF8)
	chtag -t -c UTF-8 $(TEST_ZDS_UTF8)

$(TEST_ZJB_UTF8): $(TEST_ZJB)
	@echo "Converting job tests..."
	iconv -f IBM-1047 -t UTF-8 $(TEST_ZJB) > $(TEST_ZJB_UTF8)
	chtag -t -c UTF-8 $(TEST_ZJB_UTF8)

$(TEST_ZUSF_UTF8): $(TEST_ZUSF)
	@echo "Converting USS tests..."
	iconv -f IBM-1047 -t UTF-8 $(TEST_ZUSF) > $(TEST_ZUSF_UTF8)
	chtag -t -c UTF-8 $(TEST_ZUSF_UTF8)

# Individual test targets
.PHONY: test-zds
test-zds: $(TEST_ZDS_UTF8)
	@echo "Running dataset tests only..."
	export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZDS_UTF8) -v

.PHONY: test-zjb
test-zjb: $(TEST_ZJB_UTF8)
	@echo "Running job tests only..."
	export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZJB_UTF8) -v

.PHONY: test-zusf
test-zusf: $(TEST_ZUSF_UTF8)
	@echo "Running USS tests only..."
	export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZUSF_UTF8) -v

# Clean all generated test result files
.PHONY: clean-results
clean-results:
	rm -f test_results_*.txt

# Show help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all         - Run all tests (default)"
	@echo "  test        - Run all tests with results saved to file"
	@echo "  test-zds    - Run dataset tests only"
	@echo "  test-zjb    - Run job tests only"
	@echo "  test-zusf   - Run USS tests only"
	@echo "  clean-results - Remove all test result files"
	@echo "  help        - Show this help message"

# Force rebuild of UTF-8 files
.PHONY: rebuild
rebuild: test