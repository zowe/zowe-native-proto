RESULTS_FILE := test_results.txt
TEST_ZDS := test_zds.py
TEST_ZJB := test_zjb.py
TEST_ZUSF := test_zusf.py
TEST_ZDS_UTF8 := test_zds_utf8.py
TEST_ZJB_UTF8 := test_zjb_utf8.py
TEST_ZUSF_UTF8 := test_zusf_utf8.py

# Check if -z flag is used (transcribe mode)
ifdef MAKEFLAGS
ifeq ($(findstring z,$(MAKEFLAGS)),z)
TRANSCRIBE_MODE := true
endif
endif

.PHONY: all
all: test

.PHONY: test
test:
	@echo "=== Test Results - $$(date) ===" | tee $(RESULTS_FILE)
	@echo | tee -a $(RESULTS_FILE)
ifdef TRANSCRIBE_MODE
	@echo "Running in transcribe mode (EBCDIC to UTF-8 conversion)" | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zds-transcribe 2>&1 | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zjb-transcribe 2>&1 | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zusf-transcribe 2>&1 | tee -a $(RESULTS_FILE)
else
	@echo "Running in direct mode (no conversion)" | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zds-direct 2>&1 | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zjb-direct 2>&1 | tee -a $(RESULTS_FILE)
	@$(MAKE) test-zusf-direct 2>&1 | tee -a $(RESULTS_FILE)
endif
	@echo | tee -a $(RESULTS_FILE)
	@echo "=== Test Completed - $$(date) ===" | tee -a $(RESULTS_FILE)
	@echo "Results saved to: $(RESULTS_FILE)"

# Convert EBCDIC to UTF-8
$(TEST_ZDS_UTF8): $(TEST_ZDS)
	@echo "Converting $(TEST_ZDS) from EBCDIC to UTF-8..."
	@iconv -f IBM-1047 -t UTF-8 $(TEST_ZDS) > $(TEST_ZDS_UTF8)
	@chtag -t -c UTF-8 $(TEST_ZDS_UTF8)

$(TEST_ZJB_UTF8): $(TEST_ZJB)
	@echo "Converting $(TEST_ZJB) from EBCDIC to UTF-8..."
	@iconv -f IBM-1047 -t UTF-8 $(TEST_ZJB) > $(TEST_ZJB_UTF8)
	@chtag -t -c UTF-8 $(TEST_ZJB_UTF8)

$(TEST_ZUSF_UTF8): $(TEST_ZUSF)
	@echo "Converting $(TEST_ZUSF) from EBCDIC to UTF-8..."
	@iconv -f IBM-1047 -t UTF-8 $(TEST_ZUSF) > $(TEST_ZUSF_UTF8)
	@chtag -t -c UTF-8 $(TEST_ZUSF_UTF8)

# Direct test targets (no conversion)
.PHONY: test-zds-direct
test-zds-direct:
	@echo "Running dataset tests (direct)..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZDS) -v

.PHONY: test-zjb-direct
test-zjb-direct:
	@echo "Running job tests (direct)..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZJB) -v

.PHONY: test-zusf-direct
test-zusf-direct:
	@echo "Running USS tests (direct)..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZUSF) -v

# Transcribe test targets (with conversion)
.PHONY: test-zds-transcribe
test-zds-transcribe: $(TEST_ZDS_UTF8)
	@echo "Running dataset tests (transcribed)..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZDS_UTF8) -v

.PHONY: test-zjb-transcribe
test-zjb-transcribe: $(TEST_ZJB_UTF8)
	@echo "Running job tests (transcribed)..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZJB_UTF8) -v

.PHONY: test-zusf-transcribe
test-zusf-transcribe: $(TEST_ZUSF_UTF8)
	@echo "Running USS tests (transcribed)..."
	@export PYTHONIOENCODING=utf-8 && pytest $(TEST_ZUSF_UTF8) -v

# Individual test targets (backward compatibility)
.PHONY: test-zds test-zjb test-zusf
test-zds:
ifdef TRANSCRIBE_MODE
	@$(MAKE) test-zds-transcribe
else
	@$(MAKE) test-zds-direct
endif

test-zjb:
ifdef TRANSCRIBE_MODE
	@$(MAKE) test-zjb-transcribe
else
	@$(MAKE) test-zjb-direct
endif

test-zusf:
ifdef TRANSCRIBE_MODE
	@$(MAKE) test-zusf-transcribe
else
	@$(MAKE) test-zusf-direct
endif

# Clean and utility targets
.PHONY: clean-results clean-utf8 clean rebuild help
clean-results:
	@rm -f test_results*.txt

clean-utf8:
	@rm -f $(TEST_ZDS_UTF8) $(TEST_ZJB_UTF8) $(TEST_ZUSF_UTF8)

clean: clean-results clean-utf8

rebuild: clean test

help:
	@echo "Available targets:"
	@echo "  make         - Run all tests directly (no EBCDIC conversion)"
	@echo "  make -z      - Run all tests with EBCDIC to UTF-8 conversion"
	@echo "  test-zds     - Run dataset tests only"
	@echo "  test-zjb     - Run job tests only" 
	@echo "  test-zusf    - Run USS tests only"
	@echo "  clean-results - Remove test result files"
	@echo "  clean-utf8   - Remove converted UTF-8 test files"
	@echo "  clean        - Remove all generated files"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make           # Direct pytest execution"
	@echo "  make -z        # Convert EBCDIC to UTF-8, then pytest"
	@echo "  make test-zds  # Run only dataset tests"
	@echo "  make -z test-zds # Convert and run dataset tests"