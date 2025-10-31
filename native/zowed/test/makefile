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

CXX=xlclang++

OUT_DIR=build-out
TARGET=$(OUT_DIR)/zowed_test_runner

INCLUDES=-I../../c/test -I../../c -I..
CXXFLAGS=-W "c,lp64,langlvl(extended0x),xplink" -c $(INCLUDES)
LDFLAGS=-W "l,lp64,xplink,map"

TEST_SOURCES=\
 zowed_test_runner.cpp \
 worker.test.cpp \
 zowed.test.cpp

TEST_OBJECTS=$(addprefix $(OUT_DIR)/,$(TEST_SOURCES:.cpp=.o))

.PHONY: all clean test

all: $(TARGET)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT_DIR)/%.o: %.cpp | $(OUT_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(TEST_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ > $(OUT_DIR)/zowed_test_runner.bind.lst

test: all
	_CEE_RUNOPTS="TRAP(ON,NOSPIE)" ./$(TARGET)

clean:
	rm -rf $(OUT_DIR)
