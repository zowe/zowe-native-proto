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
ASM=as
BIND=ld

# https://www.ibm.com/docs/en/zos/3.1.0?topic=bor-binder-options
# https://www.ibm.com/docs/en/zos/2.2.0?topic=descriptions-ld-link-object-files
BIND_FLAGS = -bRMODE=ANY
BIND_FLAGS_64 = -bRMODE=ANY -bAMODE=64

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
 list,\
 showmacro,\
 aggregate

MTL_OPTS64=$(MTL_OPTS),lp64
MTL_FLAGS=-S -W "c,$(MTL_OPTS)"
MTL_FLAGS64=-S -W "c,$(MTL_OPTS64)"

MACLIBS=-ISYS1.MACLIB \
 -ISYS1.MODGEN \
 -ICBC.SCCNSAM

MTL_HEADERS=-I/usr/include/metal

CPP_BND_FLAGS=-W "l,lp64,xplink,map,list"

CPP_BND_FLAGS_AUTH=-W "l,lp64,xplink,map,list,ac=1"

C_FLAGS=-W "c,lp64,langlvl(extended),xplink,exportall"\
 -c \
 -I./chdsect

DLL_CPP_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink,exportall"\
 -c \
 -I./chdsect

DLL_BND_FLAGS=-W "l,lp64,dll,dynam=dll,xplink,map,list"

CPP_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink"\
 -c

ASM_FLAGS=-mRENT

# make -DBuildType=DEBUG
.IF $(BuildType) == DEBUG
# -g generates debugging information
DEBUGGER_FLAGS=-qSOURCE -qDEBUG
CPP_BND_FLAGS+=$(DEBUGGER_FLAGS)
CPP_BND_FLAGS_AUTH+=$(DEBUGGER_FLAGS)
C_FLAGS+=$(DEBUGGER_FLAGS)
DLL_BND_FLAGS+=$(DEBUGGER_FLAGS)
DLL_CPP_FLAGS+=$(DEBUGGER_FLAGS) -qSHOWINC
CPP_FLAGS+=$(DEBUGGER_FLAGS) -qSHOWINC
MTL_FLAGS+=$(DEBUGGER_FLAGS) -qSHOWINC
MTL_FLAGS64+=$(DEBUGGER_FLAGS) -qSHOWINC
ASM_FLAGS+=--verbose
.ELSE
PRODUCTION_FLAGS=-qNOSOURCE -qNODEBUG
CPP_BND_FLAGS+=$(PRODUCTION_FLAGS)
CPP_BND_FLAGS_AUTH+=$(PRODUCTION_FLAGS)
C_FLAGS+=$(PRODUCTION_FLAGS)
DLL_BND_FLAGS+=$(PRODUCTION_FLAGS)
DLL_CPP_FLAGS+=$(PRODUCTION_FLAGS) -qNOSHOWINC
CPP_FLAGS+=$(PRODUCTION_FLAGS) -qNOSHOWINC
MTL_FLAGS+=$(PRODUCTION_FLAGS) -qNOSHOWINC
MTL_FLAGS64+=$(PRODUCTION_FLAGS) -qNOSHOWINC
ASM_FLAGS+=--noverbose
.END


all: testrcvy testasm1 testasm4

#
# Testing utilities
#
testrcvy.s: testrcvy.c
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -I../chdsect -I../ -o $@ $^

testrcvy.o: testrcvy.s
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

testrcvy: testrcvy.o
	$(BIND) $(BIND_FLAGS_64) -V -eMAIN -o $@ $^ > $*.bind.lst

testasm1.o: testasm1.s
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

testasm4.o: testasm4.s
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

testasm1: testasm1.o
	$(BIND) $(BIND_FLAGS) -V -eTESTASM1 -o $@ $^ > $*.bind.lst

testasm4: testasm4.o
	$(BIND) $(BIND_FLAGS_64) -V -eTESTASM4 -o $@ $^ > $*.bind.lst

clean:
	rm -f *.o
	rm -f *.dbg
	rm -f *.so
	rm -f *.a
	rm -f *.u
	rm -f *.x
	rm -f *.lst
	rm -f CEEDUMP.*
