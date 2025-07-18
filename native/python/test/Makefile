RESULTS_FILE := test_results.txt
TEST_ZDS := test_zds.py
TEST_ZJB := test_zjb.py
TEST_ZUSF := test_zusf.py
TEST_ZDS_UTF8 := test_zds_utf8.py
TEST_ZJB_UTF8 := test_zjb_utf8.py
TEST_ZUSF_UTF8 := test_zusf_utf8.py
PY_ENV := export PYTHONIOENCODING=utf-8 && ./venv
PIP := $(PY_ENV)/bin/pip
PYTEST := $(PY_ENV)/bin/pytest

all: test

test: setup
	@echo "=== Test Results - $$(date) ===" | tee $(RESULTS_FILE)
	@echo | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zds 2>&1 | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zjb 2>&1 | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zusf 2>&1 | tee -a $(RESULTS_FILE)
	@echo | tee -a $(RESULTS_FILE)
	@echo "=== Test Completed - $$(date) ===" | tee -a $(RESULTS_FILE)
	@echo "Results saved to: $(RESULTS_FILE)"
	@$(MAKE) teardown

# Pattern rule to convert EBCDIC to UTF-8
%_utf8.py: %.py
	@iconv -f IBM-1047 -t UTF-8 $< > $@
	@chtag -t -c UTF-8 $@

# Individual test targets
test-zds: $(TEST_ZDS_UTF8)
	@echo "Running dataset tests..."
	@$(PYTEST) $(TEST_ZDS_UTF8) -v

test-zjb: $(TEST_ZJB_UTF8)
	@echo "Running job tests..."
	@$(PYTEST) $(TEST_ZJB_UTF8) -v

test-zusf: $(TEST_ZUSF_UTF8)
	@echo "Running USS tests..."
	@$(PYTEST) $(TEST_ZUSF_UTF8) -v

# Utility targets
setup:
	@iconv -f IBM-1047 -t UTF-8 requirements.txt > requirements_utf8.txt
	@chtag -t -c UTF-8 requirements_utf8.txt
	@python -m venv venv
	@$(PIP) install -r requirements_utf8.txt

teardown:
	@rm -rf __pycache__ venv requirements_utf8.txt

clean:
	$(MAKE) teardown
	@rm -f test_results*.txt $(TEST_ZDS_UTF8) $(TEST_ZJB_UTF8) $(TEST_ZUSF_UTF8)

help:
	@echo "Available targets:"
	@echo "  all         - Run all tests with results saved to file"
	@echo "  test-zds    - Run dataset tests only"
	@echo "  test-zjb    - Run job tests only"
	@echo "  test-zusf   - Run USS tests only"
	@echo "  clean 		 - Remove all test result files"
	@echo "  help        - Show this help message"

.PHONY: all test-zds test-zjb test-zusf clean-results help clean setup teardown
