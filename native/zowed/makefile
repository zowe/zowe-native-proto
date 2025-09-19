# Makefile for zowed C++ implementation
# This program and the accompanying materials are made available under the terms of the
# Eclipse Public License v2.0 which accompanies this distribution, and is available at
# https://www.eclipse.org/legal/epl-v20.html
#
# SPDX-License-Identifier: EPL-2.0
#
# Copyright Contributors to the Zowe Project.

# Compilers and flags
CXX_LIB = xlclang++
CXX_MAIN = xlc++
CC = xlc
ASM = as

CXXFLAGS_LIB = -std=c++11 -Wall -Wextra -O2 -qlanglvl=extended0x -I../c -W "c,exportall,dll"
CXXFLAGS_MAIN = -qlanglvl=extended0x -O2 -I../c -W "c,lp64,xplink"
LDFLAGS = -W "l,lp64,xplink,map,list"
DLL_BND_FLAGS = -W "l,lp64,dll,dynam=dll,xplink,map,list"

MTL_OPTS=metal,\
 langlvl(extended),\
 sscom,\
 nolongname,\
 inline,\
 genasm,\
 inlrpt,\
 csect,\
 nose,\
 list,\
 warn64,\
 optimize(2),\
 aggregate

MTL_OPTS64=$(MTL_OPTS),lp64
MTL_FLAGS=-S -W "c,$(MTL_OPTS)"
MTL_FLAGS64=-S -W "c,$(MTL_OPTS64)"

MACLIBS= -I../asmmac \
 -ISYS1.MACLIB \
 -ISYS1.MODGEN \
 -ICBC.SCCNSAM

MTL_HEADERS=-I/usr/include/metal \
 -I/usr/include \
 -I../c

ASM_FLAGS=-mRENT

# Target executable and library
OUT_DIR = build-out
TARGET = zowed
LIBRARY = libzowed.so

# Source files
LIB_SOURCES = zowed.cpp worker.cpp cmds/ds.cpp
LIB_OBJECTS = $(OUT_DIR)/zowed.lib.o $(OUT_DIR)/worker.lib.o $(OUT_DIR)/cmds/ds.lib.o

MAIN_SOURCES = main.cpp
MAIN_OBJECTS = $(OUT_DIR)/main.o

# Header files
HEADERS = worker.hpp cmds/ds.hpp zowed.hpp

# Default target - build both shared library and executable
all: $(OUT_DIR)/$(LIBRARY) $(OUT_DIR)/$(TARGET)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)
	mkdir -p $(OUT_DIR)/cmds

# Build the shared library (.so file)
$(OUT_DIR)/$(LIBRARY): $(OUT_DIR) $(LIB_OBJECTS) $(OUT_DIR)/zjsonm.o
	$(CXX_LIB) $(DLL_BND_FLAGS) -o $(OUT_DIR)/$(LIBRARY) $(LIB_OBJECTS) ../c/build-out/xlclang/zds.o ../c/build-out/xlclang/zut.o ../c/build-out/zam.o ../c/build-out/zdsm.o ../c/build-out/zutm.o ../c/build-out/zutm31.o $(OUT_DIR)/zjsonm.o > $(OUT_DIR)/libzowed.bind.lst

# Build the executable (main compiled with xlc++, dynamically loads shared library)
$(OUT_DIR)/$(TARGET): $(OUT_DIR) $(MAIN_OBJECTS)
	$(CXX_MAIN) $(LDFLAGS) -o $(OUT_DIR)/$(TARGET) $(MAIN_OBJECTS) > $(OUT_DIR)/zowed.bind.lst

# Compile library source files with xlclang++
$(OUT_DIR)/%.lib.o: %.cpp $(HEADERS) $(OUT_DIR)
	$(CXX_LIB) $(CXXFLAGS_LIB) -c $< -o $@

# Compile main source files with xlc++
$(OUT_DIR)/%.o: %.cpp $(HEADERS) $(OUT_DIR)
	$(CXX_MAIN) $(CXXFLAGS_MAIN) -c $< -o $@

# Compile zjsonm Metal C source to assembly
$(OUT_DIR)/zjsonm.s: ../c/zjsonm.c $(OUT_DIR)
	$(CC) $(MTL_FLAGS64) -qlist=$(OUT_DIR)/zjsonm.mtl.lst $(MTL_HEADERS) -o $@ $<

# Assemble zjsonm assembly to object
$(OUT_DIR)/zjsonm.o: $(OUT_DIR)/zjsonm.s
	$(ASM) $(ASM_FLAGS) -a=$(OUT_DIR)/zjsonm.s.lst $(MACLIBS) -o $@ $<

# Clean build artifacts
clean:
	rm -rf $(OUT_DIR)

# Library only target
library: $(OUT_DIR)/$(LIBRARY)

# Debug target with additional flags
debug: CXXFLAGS_LIB += -g -DDEBUG
debug: CXXFLAGS_MAIN += -g -DDEBUG
debug: $(OUT_DIR)/$(TARGET)

# Test target - placeholder
test:
	@echo "No tests configured yet"

# Phony targets
.PHONY: all clean test debug help library

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build libzowed shared library and zowed executable (default)"
	@echo "  library - Build only the libzowed.so shared library"
	@echo "  clean   - Remove build artifacts from $(OUT_DIR)"
	@echo "  debug   - Build with debug symbols"
	@echo "  test    - Run tests (placeholder)"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Build approach:"
	@echo "  - $(TARGET) uses dlopen() to load $(LIBRARY) at runtime"
	@echo ""
	@echo "Compilers:"
	@echo "  - Shared Library ($(LIBRARY)) compiled with $(CXX_LIB)"
	@echo "  - Main executable ($(TARGET)) compiled with $(CXX_MAIN)"
