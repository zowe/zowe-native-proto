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
OUT_DIRS=$(OUT_DIR_CXXLANG)

XLC_FLAGS=_CC_ACCEPTABLE_RC=0 _C89_ACCEPTABLE_RC=0 _CXX_ACCEPTABLE_RC=0
CXX=$(XLC_FLAGS) xlc++
CC=$(XLC_FLAGS) xlc
ASM=as
BIND=ld

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

MACLIBS= -I../../../asmmac \
 -ISYS1.MACLIB \
 -ISYS1.MODGEN \
 -ICBC.SCCNSAM

MTL_HEADERS=-I/usr/include/metal \
 -I../../chdsect \
 -I../../../c

CPP_BND_FLAGS=-W "l,lp64,xplink,map,list"

CPP_BND_FLAGS_AUTH=-W "l,lp64,xplink,map,list,ac=1"

C_FLAGS=-W "c,lp64,langlvl(extended),xplink,exportall"\
 -c \
 -I../../chdsect \
 -I../../../c

DLL_CPP_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink,exportall"\
 -c \
 -I./chdsect

DLL_BND_FLAGS=-W "l,lp64,dll,dynam=dll,xplink,map,list"

CPP_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink"\
 -c \
 -I../../chdsect \
 -I../../../c

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

all: build-out build-out/ams

ams: build-out/ams

build-out:
	@echo 'Creating build-out'
	mkdir -p build-out

build-out/ams.s: ams.c
	@echo 'Building build-out/ams.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/ams.o: build-out/ams.s
	@echo 'Building build-out/ams.s'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/ams: build-out/ams.o
	$(BIND) $(BIND_FLAGS_64) -V -eMAIN -o $@ $^ > $*.bind.lst

clean:
	rm -f *.o
	rm -f *.dbg
	rm -f *.so
	rm -f *.a
	rm -f *.x
	rm -f *.lst
	rm -f CEEDUMP.*
	rm -rf build-out
