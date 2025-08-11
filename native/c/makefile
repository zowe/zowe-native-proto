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

XLC_FLAGS=_CC_ACCEPTABLE_RC=0 _C89_ACCEPTABLE_RC=0 _CXX_ACCEPTABLE_RC=0
CXX=$(XLC_FLAGS) xlc++
CXXLANG=xlclang++
CC=$(XLC_FLAGS) xlc
ASM=as

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
 -I./chdsect

CPP_BND_FLAGS=-W "l,lp64,xplink,map,list"

CPP_BND_FLAGS_AUTH=-W "l,lp64,xplink,map,list,ac=1"

C_FLAGS=-W "c,lp64,langlvl(extended),xplink,exportall"\
 -c \
 -I./chdsect

CXXLANG_FLAGS=-W "c,lp64"\
 -c \
 -I./chdsect

SWIG_FLAGS=-W "c,lp64,define(SWIG)"\
 -c \
 -I./chdsect \
 -DSWIG

DLL_CPP_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink,exportall"\
 -c \
 -I./chdsect

DLL_BND_FLAGS=-W "l,lp64,dll,dynam=dll,xplink,map,list"

CPP_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink"\
 -c -D__IBMCPP_TR1__=1

ASM_FLAGS=-mRENT

# make -DBuildType=DEBUG
.IF $(BuildType) == DEBUG
# -g generates debugging information
DEBUGGER_FLAGS=-qSOURCE -g9
OTHER_C_FLAGS=-qSHOWINC -qSHOWMACROS

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
CPP_BND_FLAGS+=-g0
CPP_BND_FLAGS_AUTH+=-g0
DLL_BND_FLAGS+=-g0
DLL_CPP_FLAGS+=-g0
ASM_FLAGS+=--noverbose

# Still investigating why we need `-g1` for the MetalC flags :'(
MTL_FLAGS+=-g1
MTL_FLAGS64+=-g1
.END

all: libzut.so libzut.a libzds.so libzds.a libzusf.so libzusf.a libzcn.so libzcn.a libzjb.so libzjb.a zowex zoweax extattr
xlclang-extenders: $(OUT_DIR_CXXLANG)/zut.o $(OUT_DIR_CXXLANG)/zds.o $(OUT_DIR_CXXLANG)/zjb.o $(OUT_DIR_CXXLANG)/zcn.o $(OUT_DIR_CXXLANG)/zusf.o $(OUT_DIR_CXXLANG)/zshmem.o
# SWIG extenders: compile C++ files with SWIG macro defined for Python bindings
swig-extenders: $(OUT_DIR_SWIG)/zut.o $(OUT_DIR_SWIG)/zds.o $(OUT_DIR_SWIG)/zjb.o $(OUT_DIR_SWIG)/zcn.o $(OUT_DIR_SWIG)/zusf.o $(OUT_DIR_SWIG)/ztso.o
tests:
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

$(OUT_DIR)/libzut.so: $(OUT_DIR)/zut.o $(OUT_DIR)/zutm.o $(OUT_DIR)/zam.o $(OUT_DIR)/zutm31.o
	@echo 'Building $(OUT_DIR)/libzut.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libzut.so: $(OUT_DIRS) $(OUT_DIR)/libzut.so

$(OUT_DIR)/libzut.a: $(OUT_DIR)/zut.o $(OUT_DIR)/zutm.o $(OUT_DIR)/zam.o $(OUT_DIR)/zutm31.o
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
# Shared memory
#
$(OUT_DIR)/zshmem.o: zshmem.cpp
	@echo 'Building $(OUT_DIR)/zshmem.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_CXXLANG)/zshmem.o: $(OUT_DIRS) zshmem.cpp
	@echo '$(OUT_DIR_CXXLANG)/zshmem.o'
	$(CXXLANG) $(CXXLANG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_SWIG)/zshmem.o: $(OUT_DIRS) zshmem.cpp
	@echo 'Building $(OUT_DIR_SWIG)/zshmem.o with SWIG macro'
	$(CXXLANG) $(SWIG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR)/libzshmem.so: $(OUT_DIR)/zshmem.o
	@echo 'Building $(OUT_DIR)/libzshmem.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

$(OUT_DIR)/libzshmem.a: $(OUT_DIR)/zshmem.o
	@echo 'Building $(OUT_DIR)/libzshmem.a'
	ar -rv $@ $^

libzshmem.so: $(OUT_DIRS) $(OUT_DIR)/libzshmem.so
libzshmem.a: $(OUT_DIRS) $(OUT_DIR)/libzshmem.a

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

$(OUT_DIR)/libzusf.so: $(OUT_DIR)/zusf.o $(OUT_DIR)/libzut.a $(OUT_DIR)/libzshmem.a
	@echo 'Building $(OUT_DIR)/libzusf.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libzusf.so: $(OUT_DIRS) $(OUT_DIR)/libzusf.so

$(OUT_DIR)/libzusf.a: $(OUT_DIR)/zusf.o $(OUT_DIR)/libzut.a $(OUT_DIR)/libzshmem.a
	@echo 'Building $(OUT_DIR)/libzusf.a'
	ar -rv $@ $^

libzusf.a: $(OUT_DIRS) $(OUT_DIR)/libzusf.a

#
# TSO
#
$(OUT_DIR)/ztso.o: ztso.cpp
	@echo 'Building $(OUT_DIR)/ztso.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR_SWIG)/ztso.o: $(OUT_DIRS) ztso.cpp
	@echo 'Building $(OUT_DIR_SWIG)/ztso.o with SWIG macro'
	$(CXXLANG) $(SWIG_FLAGS) -qlist=$*.cpp.lst -o $@ $^

$(OUT_DIR)/libztso.so: ztso
	@echo 'Building $(OUT_DIR)/libztso.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libztso.so: $(OUT_DIRS) $(OUT_DIR)/libztso.so

$(OUT_DIR)/libztso.a: $(OUT_DIR)/ztso.o
	@echo 'Building $(OUT_DIR)/libztso.a'
	ar -rv $@ $^

libztso.a: $(OUT_DIRS) $(OUT_DIR)/libztso.a

#
# Test CLI
#
# zowex: zowex.o zcn.o zcnm.o zcnm31.o zut.o zutm.o zutm31.o zjb.o zjbm.o zssi31.o zjbm31.o zds.o
$(OUT_DIR)/zowex: $(OUT_DIR)/zowex.o $(OUT_DIR)/libzcn.a $(OUT_DIR)/libzut.a $(OUT_DIR)/libzjb.a $(OUT_DIR)/libzds.a $(OUT_DIR)/libzusf.a $(OUT_DIR)/libztso.a $(OUT_DIR)/libzshmem.a
	@echo 'Building zowex'
	$(CXX) $(CPP_BND_FLAGS) -o $@ $^ > $*.bind.lst
	ln -sf ../c/$@ ../golang/zowex
	@echo 'Copied zowex to golang/'

zowex: $(OUT_DIRS) $(OUT_DIR)/zowex

$(OUT_DIR)/zoweax: $(OUT_DIR)/zowex.o $(OUT_DIR)/libzcn.a $(OUT_DIR)/libzut.a $(OUT_DIR)/libzjb.a $(OUT_DIR)/libzds.a $(OUT_DIR)/libzusf.a $(OUT_DIR)/libztso.a $(OUT_DIR)/libzshmem.a
	@echo 'Building zoweax'
	$(CXX) $(CPP_BND_FLAGS_AUTH) -o $@ $^ > $*.bind.lst
	ln -sf ../c/$@ ../golang/zoweax
	@echo 'Copied zoweax to golang/'

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
extattr:
	extattr +ap $(OUT_DIR)/zoweax

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
