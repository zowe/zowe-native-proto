# Makefile for running Flask route tests
# This Makefile runs all test files and saves results to text files

# Variables
PYTHON := python3
PYTEST := pytest
TEST_DIR := test
RESULTS_DIR := test_results
TIMESTAMP := $(shell date +%Y%m%d_%H%M%S)

# Test files
ZUSF_TEST := $(TEST_DIR)/test_zusf_routes.py
ZDS_TEST := $(TEST_DIR)/test_zds_routes.py
ZJB_TEST := $(TEST_DIR)/test_zjb_routes.py

# Result files
ZUSF_RESULT := $(RESULTS_DIR)/zusf_test_results_$(TIMESTAMP).txt
ZDS_RESULT := $(RESULTS_DIR)/zds_test_results_$(TIMESTAMP).txt
ZJB_RESULT := $(RESULTS_DIR)/zjb_test_results_$(TIMESTAMP).txt
ALL_RESULT := $(RESULTS_DIR)/all_test_results_$(TIMESTAMP).txt

# Default target
.PHONY: all
all: setup test-all

# Setup - create results directory
.PHONY: setup
setup:
	@echo "Setting up test environment..."
	@mkdir -p $(RESULTS_DIR)
	@echo "Results directory created: $(RESULTS_DIR)"

# Install dependencies
.PHONY: install
install:
	@echo "Installing test dependencies..."
	@$(PYTHON) -m pip install pytest flask

# Run all tests
.PHONY: test-all
test-all: setup test-zusf test-zds test-zjb combine-results
	@echo "All tests completed. Results saved to $(ALL_RESULT)"

# Run ZUSF tests
.PHONY: test-zusf
test-zusf: setup
	@echo "Running ZUSF route tests..."
	@echo "=== ZUSF Route Tests - $(shell date) ===" > $(ZUSF_RESULT)
	@echo "" >> $(ZUSF_RESULT)
	@$(PYTEST) $(ZUSF_TEST) -v --tb=short >> $(ZUSF_RESULT) 2>&1 || true
	@echo "" >> $(ZUSF_RESULT)
	@echo "=== Test completed at $(shell date) ===" >> $(ZUSF_RESULT)
	@echo "ZUSF tests completed. Results saved to $(ZUSF_RESULT)"

# Run ZDS tests
.PHONY: test-zds
test-zds: setup
	@echo "Running ZDS route tests..."
	@echo "=== ZDS Route Tests - $(shell date) ===" > $(ZDS_RESULT)
	@echo "" >> $(ZDS_RESULT)
	@$(PYTEST) $(ZDS_TEST) -v --tb=short >> $(ZDS_RESULT) 2>&1 || true
	@echo "" >> $(ZDS_RESULT)
	@echo "=== Test completed at $(shell date) ===" >> $(ZDS_RESULT)
	@echo "ZDS tests completed. Results saved to $(ZDS_RESULT)"

# Run ZJB tests
.PHONY: test-zjb
test-zjb: setup
	@echo "Running ZJB route tests..."
	@echo "=== ZJB Route Tests - $(shell date) ===" > $(ZJB_RESULT)
	@echo "" >> $(ZJB_RESULT)
	@$(PYTEST) $(ZJB_TEST) -v --tb=short >> $(ZJB_RESULT) 2>&1 || true
	@echo "" >> $(ZJB_RESULT)
	@echo "=== Test completed at $(shell date) ===" >> $(ZJB_RESULT)
	@echo "ZJB tests completed. Results saved to $(ZJB_RESULT)"

# Combine all results into one file
.PHONY: combine-results
combine-results:
	@echo "Combining all test results..."
	@echo "=== COMBINED TEST RESULTS - $(shell date) ===" > $(ALL_RESULT)
	@echo "" >> $(ALL_RESULT)
	@echo "=== ZUSF ROUTE TESTS ===" >> $(ALL_RESULT)
	@cat $(ZUSF_RESULT) >> $(ALL_RESULT)
	@echo "" >> $(ALL_RESULT)
	@echo "=== ZDS ROUTE TESTS ===" >> $(ALL_RESULT)
	@cat $(ZDS_RESULT) >> $(ALL_RESULT)
	@echo "" >> $(ALL_RESULT)
	@echo "=== ZJB ROUTE TESTS ===" >> $(ALL_RESULT)
	@cat $(ZJB_RESULT) >> $(ALL_RESULT)
	@echo "" >> $(ALL_RESULT)
	@echo "=== SUMMARY ===" >> $(ALL_RESULT)
	@echo "Test run completed at: $(shell date)" >> $(ALL_RESULT)
	@echo "Individual result files:" >> $(ALL_RESULT)
	@echo "  - ZUSF: $(ZUSF_RESULT)" >> $(ALL_RESULT)
	@echo "  - ZDS: $(ZDS_RESULT)" >> $(ALL_RESULT)
	@echo "  - ZJB: $(ZJB_RESULT)" >> $(ALL_RESULT)

# Run tests with coverage
.PHONY: test-coverage
test-coverage: setup
	@echo "Running tests with coverage..."
	@echo "=== Test Coverage Report - $(shell date) ===" > $(RESULTS_DIR)/coverage_report_$(TIMESTAMP).txt
	@echo "" >> $(RESULTS_DIR)/coverage_report_$(TIMESTAMP).txt
	@$(PYTEST) $(TEST_DIR)/ --cov=routes --cov-report=term >> $(RESULTS_DIR)/coverage_report_$(TIMESTAMP).txt 2>&1 || true
	@echo "Coverage report saved to $(RESULTS_DIR)/coverage_report_$(TIMESTAMP).txt"

# Quick test run (less verbose)
.PHONY: test-quick
test-quick: setup
	@echo "Running quick tests..."
	@echo "=== Quick Test Run - $(shell date) ===" > $(RESULTS_DIR)/quick_test_$(TIMESTAMP).txt
	@$(PYTEST) $(TEST_DIR)/ --tb=line >> $(RESULTS_DIR)/quick_test_$(TIMESTAMP).txt 2>&1 || true
	@echo "Quick test results saved to $(RESULTS_DIR)/quick_test_$(TIMESTAMP).txt"

# Clean up result files
.PHONY: clean
clean:
	@echo "Cleaning up test result files..."
	@rm -rf $(RESULTS_DIR)
	@echo "Test results directory removed"

# Clean up Python cache
.PHONY: clean-cache
clean-cache:
	@echo "Cleaning up Python cache files..."
	@find . -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	@find . -name "*.pyc" -delete 2>/dev/null || true
	@find . -name "*.pyo" -delete 2>/dev/null || true
	@echo "Python cache cleaned"

# Full clean
.PHONY: clean-all
clean-all: clean clean-cache
	@echo "Full cleanup completed"

# Show help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all           - Run all tests and save results"
	@echo "  test-all      - Run all tests (same as 'all')"
	@echo "  test-zusf     - Run only ZUSF route tests"
	@echo "  test-zds      - Run only ZDS route tests"
	@echo "  test-zjb      - Run only ZJB route tests"
	@echo "  test-coverage - Run tests with coverage report"
	@echo "  test-quick    - Run quick tests (less verbose)"
	@echo "  install       - Install test dependencies"
	@echo "  clean         - Remove test result files"
	@echo "  clean-cache   - Remove Python cache files"
	@echo "  clean-all     - Full cleanup"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Results are saved in the $(RESULTS_DIR) directory with timestamps"

# Show latest results
.PHONY: show-results
show-results:
	@echo "Latest test results:"
	@ls -la $(RESULTS_DIR)/ | head -10

# Validate test files exist
.PHONY: validate
validate:
	@echo "Validating test files..."
	@test -f $(ZUSF_TEST) || (echo "ERROR: $(ZUSF_TEST) not found" && exit 1)
	@test -f $(ZDS_TEST) || (echo "ERROR: $(ZDS_TEST) not found" && exit 1)
	@test -f $(ZJB_TEST) || (echo "ERROR: $(ZJB_TEST) not found" && exit 1)
	@echo "All test files found"