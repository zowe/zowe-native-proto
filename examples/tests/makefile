#
#  This program and the accompanying materials are
#  made available under the terms of the Eclipse Public License v2.0
#  which accompanies this distribution, and is available at
#  https://www.eclipse.org/legal/epl-v20.html
#
#  SPDX-License-Identifier: EPL-2.0
#
#  Copyright Contributors to the Zowe Project.
#

CXX=g++
CXXFLAGS=-Wall -std=c++11
HEADER=../../native/c/test
INCLUDE=-I$(HEADER)
BUILD_DIR=build


# Default target
all: $(BUILD_DIR)/runner

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Object files
$(BUILD_DIR)/more.o: more.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(BUILD_DIR)/evenmore.o: evenmore.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(BUILD_DIR)/runner.o: runner.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# Link the runner executable
$(BUILD_DIR)/runner: $(BUILD_DIR)/runner.o $(BUILD_DIR)/more.o $(BUILD_DIR)/evenmore.o
	$(CXX) -o $@ $^

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) runner

# Utility targets
touch:
	touch testcli.cpp

chmod:
	chmod +x runner

.PHONY: all clean touch chmod