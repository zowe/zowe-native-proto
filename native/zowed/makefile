# Makefile for zowed C++ implementation
# This program and the accompanying materials are made available under the terms of the
# Eclipse Public License v2.0 which accompanies this distribution, and is available at
# https://www.eclipse.org/legal/epl-v20.html
#
# SPDX-License-Identifier: EPL-2.0
#
# Copyright Contributors to the Zowe Project.

# Compiler and flags
CXX = xlclang++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -qlanglvl=extended0x -I../c
LDFLAGS = -W "l,lp64,xplink,map,list"

# Target executable
TARGET = zowed

# Source files
SOURCES = zowed.cpp worker.cpp cmds/ds.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Header files
HEADERS = worker.hpp extern/picojson.h cmds/ds.hpp

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJECTS) ../c/build-out/xlclang/zds.o ../c/build-out/xlclang/zut.o ../c/build-out/zam.o ../c/build-out/zdsm.o ../c/build-out/zutm.o ../c/build-out/zutm31.o > $*.bind.lst

# Compile source files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)

# Install target (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Phony targets
.PHONY: all clean install test debug help

# Debug target with additional flags
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Test target - placeholder
test:
	@echo "No tests configured for picojson version"

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the zowed executable (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build with debug symbols"
	@echo "  test    - Run tests (placeholder)"
	@echo "  install - Install to /usr/local/bin"
	@echo "  help    - Show this help message"
