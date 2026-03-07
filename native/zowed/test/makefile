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

.INCLUDE: ../../c/toolchain.mk

OUT_DIR=build-out
TARGET=$(OUT_DIR)/zowed_test_runner

INCLUDES=-I../../c/test -I../../c -I..
CXXFLAGS=-c $(INCLUDES) -std=gnu++17
BND_FLAGS=$(CPP_BND_BASE_FLAGS)

.IF $(BuildType) == DEBUG
BND_FLAGS=$(CPP_BND_DEBUG_FLAGS)
.END

TEST_OBJECTS=$(OUT_DIR)/zowed_test_runner.o \
 $(OUT_DIR)/worker.test.o \
 $(OUT_DIR)/zowed.test.o

.PHONY: all clean test

all: $(TARGET)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(TEST_OBJECTS): $(OUT_DIR)

$(OUT_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(TEST_OBJECTS)
	$(CXX) $(BND_FLAGS) -o $@ $^

test: all
	_CEE_RUNOPTS="TRAP(ON,NOSPIE)" ./$(TARGET)

clean:
	rm -rf $(OUT_DIR)
