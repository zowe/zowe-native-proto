RESULTS_FILE := test_results.txt
TEST_ZDS := test_zds.py
TEST_ZJB := test_zjb.py
TEST_ZUSF := test_zusf.py
TEST_ZDS_UTF8 := test_zds_utf8.py
TEST_ZJB_UTF8 := test_zjb_utf8.py
TEST_ZUSF_UTF8 := test_zusf_utf8.py

.PHONY: all
all: test

.PHONY: test
test:
	@echo "=== Test Results - $$(date) ===" | tee $(RESULTS_FILE)
	@echo | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zds 2>&1 | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zjb 2>&1 | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zusf 2>&1 | tee -a $(RESULTS_FILE)
	@echo | tee -a $(RESULTS_FILE)
	@echo "=== Test Completed - $$(date) ===" | tee -a $(RESULTS_FILE)
	@echo "Results saved to: $(RESULTS_FILE)"

# Convert EBCDIC to UTF-8
$(TEST_ZDS_UTF8): $(TEST_ZDS)
	@iconv -f IBM-1047 -t UTF-8 $(TEST_ZDS) > $(TEST_ZDS_UTF8)
	@chtag -t -c UTF-8 $(TEST_ZDS_UTF8)

$(TEST_ZJB_UTF8): $(TEST_ZJB)
	@iconv -f IBM-1047 -t UTF-8 $(TEST_ZJB) > $(TEST_ZJB_UTF8)
	@chtag -t -c UTF-8 $(TEST_ZJB_UTF8)

$(TEST_ZUSF_UTF8): $(TEST_ZUSF)
	@iconv -f IBM-1047 -t UTF-8 $(TEST_ZUSF) > $(TEST_ZUSF_UTF8)
	@chtag -t -c UTF-8 $(TEST_ZUSF_UTF8)

# Individual test targets
.PHONY: test-zds
test-zds: $(TEST_ZDS_UTF8)
	@echo "Running dataset tests..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZDS_UTF8) -v

.PHONY: test-zjb
test-zjb: $(TEST_ZJB_UTF8)
	@echo "Running job tests..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZJB_UTF8) -v

.PHONY: test-zusf
test-zusf: $(TEST_ZUSF_UTF8)
	@echo "Running USS tests..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZUSF_UTF8) -v

# Clean and utility targets
.PHONY: clean-results rebuild help
clean-results:
	@rm -f test_results*.txt

rebuild: test

help:
	@echo "Available targets:"
	@echo "  all         - Run all tests with results saved to file"
	@echo "  test-zds    - Run dataset tests only"
	@echo "  test-zjb    - Run job tests only"
	@echo "  test-zusf   - Run USS tests only"
	@echo "  clean-results - Remove all test result files"
	@echo "  help        - Show this help message"