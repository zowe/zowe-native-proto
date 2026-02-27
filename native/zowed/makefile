# Makefile for zowed C++ implementation
# This program and the accompanying materials are made available under the terms of the
# Eclipse Public License v2.0 which accompanies this distribution, and is available at
# https://www.eclipse.org/legal/epl-v20.html
#
# SPDX-License-Identifier: EPL-2.0
#
# Copyright Contributors to the Zowe Project.

.INCLUDE: ../c/toolchain.mk

CXXFLAGS_LIB = -std=gnu++17 -Wall -Werror -MMD -MP -O2 -I../c -I../c/chdsect -fvisibility=default -fno-aligned-allocation -D_EXT -D_OPEN_SYS_FILE_EXT=1
CXXFLAGS_MAIN = -std=gnu++17 -O2 -I../c -I../c/chdsect
CPP_BND_FLAGS = $(CPP_BND_DEBUG_FLAGS)
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
CPP_OBJECTS = ../c/build-out/zds.o ../c/build-out/zjb.o \
	../c/build-out/ztso.o ../c/build-out/zusf.o ../c/build-out/zut.o
METAL_OBJECTS = ../c/build-out/zam.o ../c/build-out/zdsm.o ../c/build-out/zjbm.o \
	../c/build-out/zjbm31.o ../c/build-out/zssi31.o ../c/build-out/zutm.o ../c/build-out/zutm31.o

# Default target - build both shared library and executable
all: $(OUT_DIR)/$(LIBRARY) $(OUT_DIR)/$(TARGET)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)
	mkdir -p $(OUT_DIR)/commands

# Build the shared library (.so file)
$(OUT_DIR)/$(LIBRARY): $(OUT_DIR) $(LIB_OBJECTS) $(CMDS_OBJECTS) $(CPP_OBJECTS) $(METAL_OBJECTS) $(OUT_DIR)/zjsonm.o
	$(CXX) $(DLL_BND_FLAGS) -o $(OUT_DIR)/$(LIBRARY) $(LIB_OBJECTS) $(CMDS_OBJECTS) \
	$(CPP_OBJECTS) $(METAL_OBJECTS) $(OUT_DIR)/zjsonm.o

# Build C++ dependencies from parent makefile
cpp-deps:
	$(MAKE) -c ../c all

$(CPP_OBJECTS): cpp-deps

# Build the executable (dynamically loads shared library)
$(OUT_DIR)/$(TARGET): $(OUT_DIR) $(OUT_DIR)/main.o
	$(CXX) $(CPP_BND_FLAGS) -o $(OUT_DIR)/$(TARGET) $(OUT_DIR)/main.o

$(OUT_DIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_LIB) -c $< -o $@

$(OUT_DIR)/commands/%.o: ../c/commands/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_LIB) -c $< -o $@

$(OUT_DIR)/main.o: main.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_MAIN) -c $< -o $@

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
.PHONY: all clean debug library cpp-deps

# Include auto-generated dependency files
.INCLUDE .IGNORE: $(LIB_OBJECTS:.o=.d) $(CMDS_OBJECTS:.o=.d)
