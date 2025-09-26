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
CXX=xlc++
CC=xlc
OCXX=xlclang++

# https://www.ibm.com/docs/en/hla-and-tf/1.6?topic=options-assembler
# https://www.ibm.com/docs/en/zos/3.1.0?topic=descriptions-as-use-hlasm-assembler-produce-object-files
ASM=as
BIND=ld

# https://www.ibm.com/docs/en/zos/3.1.0?topic=bor-binder-options
# https://www.ibm.com/docs/en/zos/3.1.0?topic=descriptions-ld-link-object-files
BIND_FLAGS = -bRMODE=ANY
BIND_FLAGS_AUTH = -bRMODE=ANY,AC=1
BIND_FLAGS_64 = -bRMODE=ANY,AMODE=64
BIND_FLAGS_64_AUTH = -bRMODE=ANY,AMODE=64,AC=1

MTL_BASE_OPTS=metal,\
 langlvl(extended),\
 sscom,\
 nolongname,\
 inline,\
 genasm,\
 csect,\
 nose,\
 warn64,\
 optimize(2)
MTL_LIST_OPTS=

MACLIBS= -I../../asmmac \
 -ISYS1.MACLIB \
 -ISYS1.MODGEN \
 -ICBC.SCCNSAM

MTL_HEADERS=-I/usr/include/metal\
-I/usr/include\
-I../\
-I../chdsect

CPP_BND_BASE_FLAGS=-W "l,lp64,xplink,map"
CPP_BND_DEBUG_FLAGS=-W "l,lp64,xplink,map,list"
CPP_BND_BASE_FLAGS_AUTH=-W "l,lp64,xplink,map,ac=1"
CPP_BND_DEBUG_FLAGS_AUTH=-W "l,lp64,xplink,map,list,ac=1"

C_FLAGS=-W "c,lp64,langlvl(extended),xplink,exportall"\
 -c \
 -I./chdsect

DLL_CPP_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink,exportall"\
 -c \
 -I./chdsect

DLL_BND_BASE_FLAGS=-W "l,lp64,dll,dynam=dll,xplink,map"
DLL_BND_DEBUG_FLAGS=-W "l,lp64,dll,dynam=dll,xplink,map,list"

CPP_FLAGS=-W "c,lp64,langlvl(extended0x),xplink"\
 -c

CXXLANG_FLAGS=-W "c,lp64"\
 -c \
 -I../ \
 -I./chdsect

CXXLANG_BND_FLAGS=-W "l,lp64"

ASM_FLAGS=-mRENT

.IF $(ZLOG_ENABLE) == 1
LOG_FLAGS=-DZLOG_ENABLE
.END

# make -DBuildType=DEBUG
.IF $(BuildType) == DEBUG
LOG_FLAGS=-DZLOG_ENABLE
# -g generates debugging information
DEBUGGER_FLAGS=-qSOURCE -qDEBUG
CPP_BND_FLAGS+=$(DEBUGGER_FLAGS)
CPP_BND_FLAGS_AUTH+=$(DEBUGGER_FLAGS)
C_FLAGS+=$(DEBUGGER_FLAGS)
DLL_BND_FLAGS+=$(DEBUGGER_FLAGS)
# DLL_CPP_FLAGS+=$(DEBUGGER_FLAGS) -qSHOWINC
# CPP_FLAGS+=$(DEBUGGER_FLAGS) -qSHOWINC
MTL_FLAGS+=$(DEBUGGER_FLAGS) -qSHOWINC
MTL_FLAGS64+=$(DEBUGGER_FLAGS) -qSHOWINC
ASM_FLAGS+=--verbose
# List output flags for debugging
CPP_LIST_FLAG=-qlist=$$*.cpp.lst
MTL_LIST_FLAG=-qlist=$$*.mtl.lst
ASM_LIST_FLAG=-a=$$*.s.lst
MTL_LIST_OPTS=,inlrpt,list,showmacro,aggregate
CPP_BND_FLAGS=$(CPP_BND_DEBUG_FLAGS)
CPP_BND_FLAGS_AUTH=$(CPP_BND_DEBUG_FLAGS_AUTH)
DLL_BND_FLAGS=$(DLL_BND_DEBUG_FLAGS)
.ELSE
PRODUCTION_FLAGS=-qNOSOURCE -qNODEBUG
CPP_BND_FLAGS+=$(PRODUCTION_FLAGS)
CPP_BND_FLAGS_AUTH+=$(PRODUCTION_FLAGS)
C_FLAGS+=$(PRODUCTION_FLAGS)
DLL_BND_FLAGS+=$(PRODUCTION_FLAGS)
DLL_CPP_FLAGS+=$(PRODUCTION_FLAGS) -qNOSHOWINC
# CPP_FLAGS+=$(PRODUCTION_FLAGS) -qNOSHOWINC
MTL_FLAGS+=$(PRODUCTION_FLAGS) -qNOSHOWINC
MTL_FLAGS64+=$(PRODUCTION_FLAGS) -qNOSHOWINC
ASM_FLAGS+=--noverbose
# No list output for production builds
CPP_LIST_FLAG=
MTL_LIST_FLAG=
ASM_LIST_FLAG=
MTL_LIST_OPTS=
CPP_BND_FLAGS=$(CPP_BND_BASE_FLAGS)
CPP_BND_FLAGS_AUTH=$(CPP_BND_BASE_FLAGS_AUTH)
DLL_BND_FLAGS=$(DLL_BND_BASE_FLAGS)
.END

MTL_OPTS=$(MTL_BASE_OPTS)$(MTL_LIST_OPTS)
MTL_OPTS64=$(MTL_OPTS),lp64
MTL_FLAGS=-S -W "c,$(MTL_OPTS)"
MTL_FLAGS64=-S -W "c,$(MTL_OPTS64)"

# Apply LOG_FLAGS to all relevant compilation flags if logging is enabled
.IF DEFINED(LOG_FLAGS)
C_FLAGS+=$(LOG_FLAGS)
DLL_CPP_FLAGS+=$(LOG_FLAGS)
CPP_FLAGS+=$(LOG_FLAGS)
CXXLANG_FLAGS+=$(LOG_FLAGS)
MTL_FLAGS+=$(LOG_FLAGS)
MTL_FLAGS64+=$(LOG_FLAGS)
.END

all: 
	$(MAKE) -DZLOG_ENABLE=1 build-out build-out/ztest_runner


#
# Output directories
#
build-out:
	@echo 'Creating build-out'
	mkdir -p build-out

#
# Testing utilities
#
build-out/ztest_runner.o: ztest_runner.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/ztest_runner: build-out/ztest_runner.o \
build-out/zbase64.test.o \
build-out/zstd.test.o \
build-out/zstorage.test.o \
build-out/zstorage.metal.test.o \
build-out/zut.test.o \
build-out/zut.o \
build-out/zutm.o \
build-out/zutm31.o \
build-out/zds.test.o \
build-out/zds.o \
build-out/zdsm.o \
build-out/zam.o \
build-out/zjb.test.o \
build-out/zjb.o \
build-out/zjbm.o \
build-out/zjbm31.o \
build-out/zssi31.o \
build-out/zcn.test.o \
build-out/zcn.o \
build-out/zcnm.o \
build-out/zcnm31.o \
build-out/zrecovery.test.o \
build-out/zrecovery.metal.test.o \
build-out/zmetal.test.o \
build-out/zmetal.metal.test.o \
build-out/zusf.test.o \
build-out/zusf.o \
build-out/zshmem.o \
build-out/zlogger.test.o \
build-out/zlogger_metal.o \
build-out/zjson.test.o \
build-out/zjsonm.o
	$(OCXX) $(CXXLANG_BND_FLAGS) -o $@ $^ > $*.bind.lst

build-out/zut.o:
	ln -sf ../../build-out/xlclang/zut.o build-out/zut.o

build-out/zutm.o:
	ln -sf ../../build-out/zutm.o build-out/zutm.o

build-out/zutm31.o:
	ln -sf ../../build-out/zutm31.o build-out/zutm31.o

build-out/zds.o:
	ln -sf ../../build-out/xlclang/zds.o build-out/zds.o

build-out/zdsm.o:
	ln -sf ../../build-out/zdsm.o build-out/zdsm.o

build-out/zam.o:
	ln -sf ../../build-out/zam.o build-out/zam.o

build-out/zjb.o:
	ln -sf ../../build-out/xlclang/zjb.o build-out/zjb.o

build-out/zjbm.o:
	ln -sf ../../build-out/zjbm.o build-out/zjbm.o

build-out/zjbm31.o:
	ln -sf ../../build-out/zjbm31.o build-out/zjbm31.o

build-out/zssi31.o:
	ln -sf ../../build-out/zssi31.o build-out/zssi31.o

build-out/zcn.o:
	ln -sf ../../build-out/xlclang/zcn.o build-out/zcn.o

build-out/zcnm.o:
	ln -sf ../../build-out/zcnm.o build-out/zcnm.o

build-out/zcnm31.o:
	ln -sf ../../build-out/zcnm31.o build-out/zcnm31.o

build-out/zusf.o:
	ln -sf ../../build-out/xlclang/zusf.o build-out/zusf.o

build-out/zshmem.o:
	ln -sf ../../build-out/zshmem.o build-out/zshmem.o

build-out/zbase64.test.o: zbase64.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zstd.test.o: zstd.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zstorage.test.o: zstorage.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zstorage.metal.test.s: zstorage.metal.test.c
	$(CC) $(MTL_FLAGS64) $(MTL_LIST_FLAG) $(MTL_HEADERS) -o $@ $^

build-out/zstorage.metal.test.o: build-out/zstorage.metal.test.s
	$(ASM) $(ASM_FLAGS) $(ASM_LIST_FLAG) $(MACLIBS) -o $@ $^

build-out/zut.test.o: zut.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zjb.test.o: zjb.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zds.test.o: zds.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zcn.test.o: zcn.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zrecovery.test.o: zrecovery.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zrecovery.metal.test.s: zrecovery.metal.test.c
	$(CC) $(MTL_FLAGS64) $(MTL_LIST_FLAG) $(MTL_HEADERS) -o $@ $^

build-out/zrecovery.metal.test.o: build-out/zrecovery.metal.test.s
	$(ASM) $(ASM_FLAGS) $(ASM_LIST_FLAG) $(MACLIBS) -o $@ $^

build-out/zmetal.test.o: zmetal.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zmetal.metal.test.s: zmetal.metal.test.c
	$(CC) $(MTL_FLAGS64) $(MTL_LIST_FLAG) $(MTL_HEADERS) -o $@ $^

build-out/zmetal.metal.test.o: build-out/zmetal.metal.test.s
	$(ASM) $(ASM_FLAGS) $(ASM_LIST_FLAG) $(MACLIBS) -o $@ $^

build-out/zusf.test.o: zusf.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zlogger.test.o: zlogger.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zlogger_metal.s: ../zlogger_metal.c
	$(CC) $(MTL_FLAGS64) $(MTL_LIST_FLAG) $(MTL_HEADERS) -o $@ $^

build-out/zlogger_metal.o: build-out/zlogger_metal.s
	$(ASM) $(ASM_FLAGS) $(ASM_LIST_FLAG) $(MACLIBS) -o $@ $^

build-out/zjson.test.o: zjson.test.cpp
	$(OCXX) $(CXXLANG_FLAGS) $(CPP_LIST_FLAG) -c $^ -o $@

build-out/zjsonm.s: ../zjsonm.c
	$(CC) $(MTL_FLAGS64) $(MTL_LIST_FLAG) $(MTL_HEADERS) -o $@ $^

build-out/zjsonm.o: build-out/zjsonm.s
	$(ASM) $(ASM_FLAGS) $(ASM_LIST_FLAG) $(MACLIBS) -o $@ $^

# build-out/testrcvy.s: testrcvy.c
# 	$(CC) $(MTL_FLAGS64) $(MTL_LIST_FLAG) $(MTL_HEADERS) -o $@ $^

# build-out/testrcvy.o: build-out/testrcvy.s
# 	$(ASM) $(ASM_FLAGS) $(ASM_LIST_FLAG) $(MACLIBS) -o $@ $^

# build-out/testrcvy: build-out/testrcvy.o
# 	$(BIND) $(BIND_FLAGS_64) -V -eMAIN -o $@ $^ > $*.bind.lst

# build-out/testasm1.o: testasm1.s
# 	$(ASM) $(ASM_FLAGS) $(ASM_LIST_FLAG) $(MACLIBS) -o $@ $^

# build-out/testasm1: build-out/testasm1.o
# 	$(BIND) $(BIND_FLAGS_AUTH) -V -eTESTASM1 -o $@ $^ > $*.bind.lst

# build-out/testasm4.o: testasm4.s
# 	$(ASM) $(ASM_FLAGS) $(ASM_LIST_FLAG) $(MACLIBS) -o $@ $^

# build-out/testasm4: build-out/testasm4.o
# 	$(BIND) $(BIND_FLAGS_64) -V -eTESTASM4 -o $@ $^ > $*.bind.lst

# build-out/main.s: main.c
# 	$(CC) $(MTL_FLAGS64) $(MTL_LIST_FLAG) $(MTL_HEADERS) -I../chdsect -I../ -o $@ $^

# build-out/main.o: build-out/main.s
# 	$(ASM) $(ASM_FLAGS) $(ASM_LIST_FLAG) $(MACLIBS) -o $@ $^

# build-out/main: build-out/main.o
# 	$(BIND) $(BIND_FLAGS_64) -V -eMAIN -o $@ $^ > $*.bind.lst

#
# Testing utilities
#
test: 
	$(MAKE) -DZLOG_ENABLE=1 build-out build-out/ztest_runner
	chmod +x test.sh
	_CEE_RUNOPTS="TRAP(ON,NOSPIE)" ./build-out/ztest_runner

clean:
	rm -f *.o
	rm -f *.dbg
	rm -f *.so
	rm -f *.a
	rm -f *.u
	rm -f *.x
	rm -f *.lst
	rm -f CEEDUMP.*
	rm -rf build-out
