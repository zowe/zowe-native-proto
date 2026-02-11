# Makefile for zowed C++ implementation
# This program and the accompanying materials are made available under the terms of the
# Eclipse Public License v2.0 which accompanies this distribution, and is available at
# https://www.eclipse.org/legal/epl-v20.html
#
# SPDX-License-Identifier: EPL-2.0
#
# Copyright Contributors to the Zowe Project.

.INCLUDE: ../c/toolchain.mk

# Compilers and flags
CXX_LIB = $(CXXLANG)
CXX_MAIN = $(CXXLANG)

CXXFLAGS_LIB = -std=c++11 -Wall -Werror -MMD -MP -O2 -I../c -I../c/chdsect -fvisibility=default -fno-aligned-allocation
CXXFLAGS_MAIN = -std=c++11 -O2 -I../c -I../c/chdsect
LDFLAGS = $(CPP_BND_DEBUG_FLAGS)
DLL_BND_FLAGS = $(DLL_BND_DEBUG_FLAGS)

MACLIBS= -I../asmmac $(MACLIBS_BASE)
MTL_HEADERS=$(MTL_HEADERS_BASE) -I../c -I../c/chdsect

# Target executable and library
OUT_DIR = build-out
TARGET = zowed
LIBRARY = libzowed.so

# Object files
LIB_OBJECTS = $(OUT_DIR)/builder.o $(OUT_DIR)/commands.o $(OUT_DIR)/dispatcher.o \
	$(OUT_DIR)/rpcio.o $(OUT_DIR)/server.o $(OUT_DIR)/validator.o $(OUT_DIR)/worker.o \
	$(OUT_DIR)/zowed.o
CMDS_OBJECTS = $(OUT_DIR)/commands/ds.o $(OUT_DIR)/commands/job.o $(OUT_DIR)/commands/tso.o \
	$(OUT_DIR)/commands/uss.o
XLCLANG_OBJECTS = ../c/build-out/ibm-clang/zds.o ../c/build-out/ibm-clang/zjb.o \
	../c/build-out/ibm-clang/ztso.o ../c/build-out/ibm-clang/zusf.o ../c/build-out/ibm-clang/zut.o
METAL_OBJECTS = ../c/build-out/zam.o ../c/build-out/zdsm.o ../c/build-out/zjbm.o \
	../c/build-out/zjbm31.o ../c/build-out/zssi31.o ../c/build-out/zutm.o ../c/build-out/zutm31.o

# Default target - build both shared library and executable
all: $(OUT_DIR)/$(LIBRARY) $(OUT_DIR)/$(TARGET)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)
	mkdir -p $(OUT_DIR)/commands

# Build the shared library (.so file)
$(OUT_DIR)/$(LIBRARY): $(OUT_DIR) $(LIB_OBJECTS) $(CMDS_OBJECTS) $(XLCLANG_OBJECTS) $(METAL_OBJECTS) $(OUT_DIR)/zjsonm.o
	$(CXX_LIB) $(DLL_BND_FLAGS) -o $(OUT_DIR)/$(LIBRARY) $(LIB_OBJECTS) $(CMDS_OBJECTS) \
	$(XLCLANG_OBJECTS) $(METAL_OBJECTS) $(OUT_DIR)/zjsonm.o

# Build ibm-clang dependencies from parent makefile
xlclang-deps:
	$(MAKE) -c ../c extenders

$(XLCLANG_OBJECTS): xlclang-deps

# Build the executable (dynamically loads shared library)
$(OUT_DIR)/$(TARGET): $(OUT_DIR) $(OUT_DIR)/main.o
	$(CXX_MAIN) $(LDFLAGS) -o $(OUT_DIR)/$(TARGET) $(OUT_DIR)/main.o

# Compile library source files with ibm-clang++64
$(OUT_DIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX_LIB) $(CXXFLAGS_LIB) -c $< -o $@

# Compile commands source files with ibm-clang++64
$(OUT_DIR)/commands/%.o: ../c/commands/%.cpp
	mkdir -p $(@D)
	$(CXX_LIB) $(CXXFLAGS_LIB) -c $< -o $@

# Compile main source files with ibm-clang++64
$(OUT_DIR)/main.o: main.cpp
	mkdir -p $(@D)
	$(CXX_MAIN) $(CXXFLAGS_MAIN) -c $< -o $@

# Compile zjsonm Metal C source to assembly
$(OUT_DIR)/zjsonm.s: ../c/zjsonm.c
	@mkdir -p $(@D)
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

# Phony targets
.PHONY: all clean debug library xlclang-deps

# Include auto-generated dependency files
.INCLUDE .IGNORE: $(LIB_OBJECTS:.o=.d) $(CMDS_OBJECTS:.o=.d)
