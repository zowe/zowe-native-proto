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
CXXFLAGS_LIB = -std=c++11 -Wall -Wextra -O2 -qlanglvl=extended0x -I../c -W "c,exportall,dll"
CXXFLAGS_MAIN = -qlanglvl=extended0x -O2 -I../c -W "c,lp64,xplink"
LDFLAGS = -W "l,lp64,xplink,map,list"
DLL_BND_FLAGS = -W "l,lp64,dll,dynam=dll,xplink,map,list"

# Target executable and library
TARGET = zowed
LIBRARY = libzowed.so

# Source files
LIB_SOURCES = zowed.cpp worker.cpp cmds/ds.cpp
LIB_OBJECTS = $(LIB_SOURCES:.cpp=.lib.o)

MAIN_SOURCES = main.cpp
MAIN_OBJECTS = $(MAIN_SOURCES:.cpp=.o)

# Header files
HEADERS = worker.hpp extern/picojson.h cmds/ds.hpp zowed.hpp

# Default target - build both shared library and executable
all: $(LIBRARY) $(TARGET)

# Build the shared library (.so file)
$(LIBRARY): $(LIB_OBJECTS)
	$(CXX_LIB) $(DLL_BND_FLAGS) -o $(LIBRARY) $(LIB_OBJECTS) ../c/build-out/xlclang/zds.o ../c/build-out/xlclang/zut.o ../c/build-out/zam.o ../c/build-out/zdsm.o ../c/build-out/zutm.o ../c/build-out/zutm31.o > $*.bind.lst

# Build the executable (main compiled with xlc++, dynamically loads shared library)
$(TARGET): $(MAIN_OBJECTS)
	$(CXX_MAIN) $(LDFLAGS) -o $(TARGET) $(MAIN_OBJECTS) > $*.bind.lst

# Compile library source files with xlclang++
%.lib.o: %.cpp $(HEADERS)
	$(CXX_LIB) $(CXXFLAGS_LIB) -c $< -o $@

# Compile main source files with xlc++
%.o: %.cpp $(HEADERS)
	$(CXX_MAIN) $(CXXFLAGS_MAIN) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(LIB_OBJECTS) $(MAIN_OBJECTS) $(TARGET) $(LIBRARY) *.bind.lst

# Install target (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Phony targets
.PHONY: all clean install test debug help library

# Library only target
library: $(LIBRARY)

# Debug target with additional flags
debug: CXXFLAGS_LIB += -g -DDEBUG
debug: CXXFLAGS_MAIN += -g -DDEBUG
debug: $(TARGET)

# Test target - placeholder
test:
	@echo "No tests configured for picojson version"

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build libzowed shared library and zowed executable (default)"
	@echo "  library - Build only the libzowed.so shared library"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build with debug symbols"
	@echo "  test    - Run tests (placeholder)"
	@echo "  install - Install to /usr/local/bin"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Build approach:"
	@echo "  - $(TARGET) uses dlopen() to load $(LIBRARY) at runtime"
	@echo ""
	@echo "Compilers:"
	@echo "  - Shared Library ($(LIBRARY)) compiled with $(CXX_LIB)"
	@echo "  - Main executable ($(TARGET)) compiled with $(CXX_MAIN)"
