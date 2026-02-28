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

OUT_DIR=build-out
OUT_DIR_CXXLANG=$(OUT_DIR)/xlclang
OUT_DIR_SWIG=$(OUT_DIR)/swig
OUT_DIRS=$(OUT_DIR_CXXLANG) $(OUT_DIR_SWIG)

.INCLUDE: toolchain.mk

MACLIBS= -I../asmmac $(MACLIBS_BASE)
MTL_HEADERS=$(MTL_HEADERS_BASE) -I./chdsect

C_FLAGS=$(C_FLAGS_BASE) -I./chdsect
CXXLANG_FLAGS=$(CXXLANG_FLAGS_BASE) -I./chdsect -MMD -MP
SWIG_FLAGS=$(SWIG_FLAGS_BASE) -I./chdsect -MMD -MP
DLL_CPP_FLAGS=$(DLL_CPP_FLAGS_BASE) -I./chdsect
CPP_FLAGS=$(CPP_FLAGS_BASE) -I./chdsect

CPP_BND_FLAGS=$(CPP_BND_DEBUG_FLAGS)
CPP_BND_FLAGS_AUTH=$(CPP_BND_DEBUG_FLAGS_AUTH)
DLL_BND_FLAGS=$(DLL_BND_DEBUG_FLAGS)

# make -DBuildType=DEBUG
.IF $(BuildType) == DEBUG
CPP_BND_FLAGS+=$(DEBUGGER_FLAGS)
CPP_BND_FLAGS_AUTH+=$(DEBUGGER_FLAGS)
C_FLAGS+=$(DEBUGGER_FLAGS)
DLL_BND_FLAGS+=$(DEBUGGER_FLAGS)
DLL_CPP_FLAGS+=$(DEBUGGER_FLAGS) $(OTHER_C_FLAGS)
CPP_FLAGS+=$(DEBUGGER_FLAGS) $(OTHER_C_FLAGS)
MTL_FLAGS+=$(DEBUGGER_FLAGS) $(OTHER_C_FLAGS)
MTL_FLAGS64+=$(DEBUGGER_FLAGS) $(OTHER_C_FLAGS)
ASM_FLAGS+=--verbose
.ELSE
C_FLAGS+=-g0
CPP_FLAGS+=-g0
CPP_BND_FLAGS=$(CPP_BND_BASE_FLAGS)
CPP_BND_FLAGS_AUTH=$(CPP_BND_BASE_FLAGS_AUTH)
DLL_BND_FLAGS=$(DLL_BND_BASE_FLAGS)
DLL_CPP_FLAGS+=-g0
ASM_FLAGS+=--noverbose

# Still investigating why we need `-g1` for the MetalC flags :'(
MTL_FLAGS+=-g1
MTL_FLAGS64+=-g1
.END

# Apply LOG_FLAGS to all relevant compilation flags if logging is enabled
.IF DEFINED(LOG_FLAGS)
C_FLAGS+=$(LOG_FLAGS)
DLL_CPP_FLAGS+=$(LOG_FLAGS)
CPP_FLAGS+=$(LOG_FLAGS)
MTL_FLAGS+=$(LOG_FLAGS)
MTL_FLAGS64+=$(LOG_FLAGS)
.END

COMMAND_OBJS = $(OUT_DIR)/commands/console.o \
	$(OUT_DIR)/commands/core.o \
	$(OUT_DIR)/commands/ds.o \
	$(OUT_DIR)/commands/job.o \
	$(OUT_DIR)/commands/system.o \
	$(OUT_DIR)/commands/tso.o \
	$(OUT_DIR)/commands/uss.o \
	$(OUT_DIR)/commands/tool.o

XLCLANG_EXTENDER_OBJS = $(OUT_DIR_CXXLANG)/zut.o $(OUT_DIR_CXXLANG)/zds.o $(OUT_DIR_CXXLANG)/zjb.o $(OUT_DIR_CXXLANG)/zcn.o $(OUT_DIR_CXXLANG)/zusf.o $(OUT_DIR_CXXLANG)/ztso.o
SWIG_EXTENDER_OBJS = $(OUT_DIR_SWIG)/zut.o $(OUT_DIR_SWIG)/zds.o $(OUT_DIR_SWIG)/zjb.o $(OUT_DIR_SWIG)/zcn.o $(OUT_DIR_SWIG)/zusf.o $(OUT_DIR_SWIG)/ztso.o

all: libzut.so libzut.a libzds.so libzds.a libzusf.so libzusf.a libzcn.so libzcn.a libzjb.so libzjb.a zowex zoweax
xlclang-extenders: $(XLCLANG_EXTENDER_OBJS)
# SWIG extenders: compile C++ files with SWIG macro defined for Python bindings
swig-extenders: $(SWIG_EXTENDER_OBJS)
zowed-tests:
	 $(MAKE) -c ../zowed/test
tests: zowed-tests
	 $(MAKE) -c test

#
# Out directories
#
$(OUT_DIRS):
	@echo 'Creating $(OUT_DIR_CXXLANG)'
	mkdir -p $(OUT_DIR_CXXLANG)
	mkdir -p $(OUT_DIR_SWIG)

#
# Utilities
#
$(OUT_DIR)/zut.o: zut.cpp
	@echo 'Building $(OUT_DIR)/zut.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_CXXLANG)/zut.o: $(OUT_DIRS) zut.cpp
	@echo '$(OUT_DIR_CXXLANG)/zut.o'
	$(CXXLANG) $(CXXLANG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_SWIG)/zut.o: $(OUT_DIRS) zut.cpp
	@echo 'Building $(OUT_DIR_SWIG)/zut.o with SWIG macro'
	$(CXXLANG) $(SWIG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR)/zutm.s: zutm.c
	@echo 'Building $(OUT_DIR)/zutm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zutm.o: $(OUT_DIR)/zutm.s
	@echo 'Building $(OUT_DIR)/zutm.s'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

$(OUT_DIR)/zutm31.s: zutm31.c
	@echo 'Building $(OUT_DIR)/zutm31.s'
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zutm31.o: $(OUT_DIR)/zutm31.s
	@echo 'Building $(OUT_DIR)/zutm31.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

#
# Metal C Logger
#
.IF DEFINED(LOG_FLAGS)
$(OUT_DIR)/zlogger_metal.s: zlogger_metal.c
	@echo 'Building $(OUT_DIR)/zlogger_metal.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zlogger_metal.o: $(OUT_DIR)/zlogger_metal.s
	@echo 'Building $(OUT_DIR)/zlogger_metal.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

LIBZUT_LOGGER_OBJ=$(OUT_DIR)/zlogger_metal.o
.ELSE
LIBZUT_LOGGER_OBJ=
.END

$(OUT_DIR)/libzut.so: $(OUT_DIR)/zut.o $(OUT_DIR)/zutm.o $(OUT_DIR)/zam.o $(OUT_DIR)/zutm31.o $(LIBZUT_LOGGER_OBJ)
	@echo 'Building $(OUT_DIR)/libzut.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libzut.so: $(OUT_DIRS) $(OUT_DIR)/libzut.so

$(OUT_DIR)/libzut.a: $(OUT_DIR)/zut.o $(OUT_DIR)/zutm.o $(OUT_DIR)/zam.o $(OUT_DIR)/zutm31.o $(LIBZUT_LOGGER_OBJ)
	@echo 'Building $(OUT_DIR)/libzut.a'
	ar -rv $@ $^

libzut.a: $(OUT_DIRS) $(OUT_DIR)/libzut.a

#
# Data sets
#
$(OUT_DIR)/zam.s: zam.c
	@echo 'Building $(OUT_DIR)/zam.s'
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zam.o: $(OUT_DIR)/zam.s
	@echo 'Building $(OUT_DIR)/zdsm.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

$(OUT_DIR)/zdsm.s: zdsm.c
	@echo 'Building $(OUT_DIR)/zdsm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zdsm.o: $(OUT_DIR)/zdsm.s
	@echo 'Building $(OUT_DIR)/zdsm.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

$(OUT_DIR)/zds.o: zds.cpp
	@echo 'Building $(OUT_DIR)/zds.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_CXXLANG)/zds.o: $(OUT_DIRS) zds.cpp
	@echo '$(OUT_DIR_CXXLANG)/zds.o'
	$(CXXLANG) $(CXXLANG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_SWIG)/zds.o: $(OUT_DIRS) zds.cpp
	@echo 'Building $(OUT_DIR_SWIG)/zds.o with SWIG macro'
	$(CXXLANG) $(SWIG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR)/libzds.so: $(OUT_DIR)/zds.o $(OUT_DIR)/zdsm.o $(OUT_DIR)/zam.o $(OUT_DIR)/libzut.a
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst
	@echo 'Building $(OUT_DIR)/libzds.so'

libzds.so: $(OUT_DIRS) $(OUT_DIR)/libzds.so

$(OUT_DIR)/libzds.a: $(OUT_DIR)/zds.o $(OUT_DIR)/zdsm.o $(OUT_DIR)/zam.o $(OUT_DIR)/libzut.a
	@echo 'Building $(OUT_DIR)/libzds.a'
	ar -rv $@ $^

libzds.a: $(OUT_DIRS) $(OUT_DIR)/libzds.a

#
# Jobs
#
$(OUT_DIR)/zjbm.s: zjbm.c
	@echo 'Building $(OUT_DIR)/zjbm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zjbm.o: $(OUT_DIR)/zjbm.s
	@echo 'Building $(OUT_DIR)/zjbm.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

$(OUT_DIR)/zjb.o: zjb.cpp
	@echo 'Building $(OUT_DIR)/zjb.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_CXXLANG)/zjb.o: $(OUT_DIRS) zjb.cpp
	@echo '$(OUT_DIR_CXXLANG)/zjb.o'
	$(CXXLANG) $(CXXLANG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_SWIG)/zjb.o: $(OUT_DIRS) zjb.cpp
	@echo 'Building $(OUT_DIR_SWIG)/zjb.o with SWIG macro'
	$(CXXLANG) $(SWIG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR)/zjbm31.s: zjbm31.c
	@echo 'Building $(OUT_DIR)/zjbm31.s'
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zjbm31.o: $(OUT_DIR)/zjbm31.s
	@echo 'Building $(OUT_DIR)/zjbm31.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

$(OUT_DIR)/zssi31.s: zssi31.c
	@echo 'Building $(OUT_DIR)/zssi31.s'
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zssi31.o: $(OUT_DIR)/zssi31.s
	@echo 'Building $(OUT_DIR)/zssi31.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

$(OUT_DIR)/libzjb.so: $(OUT_DIR)/zjb.o $(OUT_DIR)/zjbm.o $(OUT_DIR)/zssi31.o $(OUT_DIR)/zjbm31.o $(OUT_DIR)/libzds.a $(OUT_DIR)/libzut.a
	@echo 'Building $(OUT_DIR)/libzjb.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libzjb.so: $(OUT_DIRS) $(OUT_DIR)/libzjb.so

$(OUT_DIR)/libzjb.a: $(OUT_DIR)/zjb.o $(OUT_DIR)/zjbm.o $(OUT_DIR)/zssi31.o $(OUT_DIR)/zjbm31.o $(OUT_DIR)/libzds.a $(OUT_DIR)/libzut.a
	@echo 'Building $(OUT_DIR)/libzjb.a'
	ar -rv $@ $^

libzjb.a: $(OUT_DIRS) $(OUT_DIR)/libzjb.a

#
# Console
#
$(OUT_DIR)/zcn.o: zcn.cpp
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_CXXLANG)/zcn.o: $(OUT_DIRS) zcn.cpp
	@echo '$(OUT_DIR_CXXLANG)/zcn.o'
	$(CXXLANG) $(CXXLANG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_SWIG)/zcn.o: $(OUT_DIRS) zcn.cpp
	@echo 'Building $(OUT_DIR_SWIG)/zcn.o with SWIG macro'
	$(CXXLANG) $(SWIG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR)/zcnm.s: zcnm.c
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zcnm.o: $(OUT_DIR)/zcnm.s
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

$(OUT_DIR)/zcnm31.s: zcnm31.c
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

$(OUT_DIR)/zcnm31.o: $(OUT_DIR)/zcnm31.s
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

$(OUT_DIR)/libzcn.so: $(OUT_DIR)/zcn.o $(OUT_DIR)/zcnm.o $(OUT_DIR)/zcnm31.o $(OUT_DIR)/libzut.a
	@echo 'Building $(OUT_DIR)/libzcn.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libzcn.so: $(OUT_DIRS) $(OUT_DIR)/libzcn.so

$(OUT_DIR)/libzcn.a: $(OUT_DIR)/zcn.o $(OUT_DIR)/zcnm.o $(OUT_DIR)/zcnm31.o $(OUT_DIR)/libzut.a
	@echo 'Building libzcn.a'
	ar -rv $@ $^

libzcn.a: $(OUT_DIRS) $(OUT_DIR)/libzcn.a

#
# USS data
#
$(OUT_DIR)/zusf.o: zusf.cpp
	@echo 'Building $(OUT_DIR)/zusf.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_CXXLANG)/zusf.o: $(OUT_DIRS) zusf.cpp
	@echo '$(OUT_DIR_CXXLANG)/zusf.o'
	$(CXXLANG) $(CXXLANG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_SWIG)/zusf.o: $(OUT_DIRS) zusf.cpp
	@echo 'Building $(OUT_DIR_SWIG)/zusf.o with SWIG macro'
	$(CXXLANG) $(SWIG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR)/libzusf.so: $(OUT_DIR)/zusf.o $(OUT_DIR)/libzut.a
	@echo 'Building $(OUT_DIR)/libzusf.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libzusf.so: $(OUT_DIRS) $(OUT_DIR)/libzusf.so

$(OUT_DIR)/libzusf.a: $(OUT_DIR)/zusf.o $(OUT_DIR)/libzut.a
	@echo 'Building $(OUT_DIR)/libzusf.a'
	ar -rv $@ $^

libzusf.a: $(OUT_DIRS) $(OUT_DIR)/libzusf.a

#
# TSO
#
$(OUT_DIR)/ztso.o: ztso.cpp
	@echo 'Building $(OUT_DIR)/ztso.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_CXXLANG)/ztso.o: $(OUT_DIRS) ztso.cpp
	@echo '$(OUT_DIR_CXXLANG)/ztso.o'
	$(CXXLANG) $(CXXLANG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_SWIG)/ztso.o: $(OUT_DIRS) ztso.cpp
	@echo 'Building $(OUT_DIR_SWIG)/ztso.o with SWIG macro'
	$(CXXLANG) $(SWIG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR)/libztso.so: ztso
	@echo 'Building $(OUT_DIR)/libztso.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libztso.so: $(OUT_DIRS) $(OUT_DIR)/libztso.so

$(OUT_DIR)/libztso.a: $(OUT_DIR)/ztso.o $(OUT_DIR)/libzut.a
	@echo 'Building $(OUT_DIR)/libztso.a'
	ar -rv $@ $^

libztso.a: $(OUT_DIRS) $(OUT_DIR)/libztso.a

#
# Commands
$(OUT_DIR)/commands/%.o: commands/%.cpp
	@echo 'Building $@'
	@mkdir -p $(@D)
	$(CXX) $(CPP_FLAGS) -qlist=$(OUT_DIR)/commands/$*.cpp.lst -c $< -o $@

#
# Extensibility
#

$(OUT_DIR)/extend/plugin.o: extend/plugin.cpp
	@echo 'Building $@'
	@mkdir -p $(@D)
	$(CXX) $(CPP_FLAGS) -qlist=$*.cpp.lst -c $< -o $@

#
# Test CLI
#
# zowex: zowex.o zcn.o zcnm.o zcnm31.o zut.o zutm.o zutm31.o zjb.o zjbm.o zssi31.o zjbm31.o zds.o
$(OUT_DIR)/zowex: $(OUT_DIR)/zowex.o $(OUT_DIR)/extend/plugin.o $(COMMAND_OBJS) $(OUT_DIR)/libzcn.a $(OUT_DIR)/libzut.a $(OUT_DIR)/libzjb.a $(OUT_DIR)/libzds.a $(OUT_DIR)/libzusf.a $(OUT_DIR)/libztso.a
	@echo 'Building zowex'
	$(CXX) $(CPP_BND_FLAGS) -o $@ $^ > $*.bind.lst

zowex: $(OUT_DIRS) $(OUT_DIR)/zowex

$(OUT_DIR)/zoweax: $(OUT_DIR)/zowex.o $(OUT_DIR)/extend/plugin.o $(COMMAND_OBJS) $(OUT_DIR)/libzcn.a $(OUT_DIR)/libzut.a $(OUT_DIR)/libzjb.a $(OUT_DIR)/libzds.a $(OUT_DIR)/libzusf.a $(OUT_DIR)/libztso.a
	@echo 'Building zoweax'
	$(CXX) $(CPP_BND_FLAGS_AUTH) -o $@ $^ > $*.bind.lst
	extattr +ap $@

zoweax: $(OUT_DIRS) $(OUT_DIR)/zoweax

$(OUT_DIR)/zowex.o: zowex.cpp
	@echo 'Building $(OUT_DIR)/zowex.o'
	@if [ -f ../package.json ]; then \
		PACKAGE_VERSION=$$(grep '"version"' ../package.json | sed 's/.*"version"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/'); \
		echo "Extracted version: $$PACKAGE_VERSION"; \
		$(CXX) $(CPP_FLAGS) -DPACKAGE_VERSION="\"$$PACKAGE_VERSION\"" -qlist=$*.cpp.lst -c $^ -o $@; \
	else \
		echo "package.json not found, building without version"; \
		$(CXX) $(CPP_FLAGS) -qlist=$*.cpp.lst -c $^ -o $@; \
	fi

#
# TODO(Kelosky): makefile for chdsect (must have ADATA in data set)
#

#
# Misc targets
#
clean:
	rm -f *.o
	rm -f *.dbg
	rm -f *.so
	rm -f *.a
	rm -f *.u
	rm -f *.x
	rm -f *.lst
	rm -f CEEDUMP.*
	rm -rf $(OUT_DIR)

# Include auto-generated dependency files for xlclang
.INCLUDE .IGNORE: $(XLCLANG_EXTENDER_OBJS:.o=.d) $(SWIG_EXTENDER_OBJS:.o=.d)
