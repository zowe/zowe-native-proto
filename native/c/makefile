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
BIND = ld

BIND_FLAGS = -bRMODE=ANY

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
 showinc,\
 showmacro,\
 source,\
 aggregate

MTL_OPTS64=$(MTL_OPTS),lp64
MTL_FLAGS=-S -W "c,$(MTL_OPTS)"
MTL_FLAGS64=-S -W "c,$(MTL_OPTS64)"

MACLIBS=-ISYS1.MACLIB \
 -ISYS1.MODGEN \
 -ICBC.SCCNSAM

MTL_HEADERS=-I/usr/include/metal \
 -I./chdsect

CPP_BND_FLAGS=-W "l,lp64,xplink,map,list"\
 -qsource \
 -g

CPP_BND_FLAGS_AUTH=-W "l,lp64,xplink,map,list,ac=1"\
 -qsource \
 -g

C_FLAGS=-W "c,lp64,langlvl(extended),xplink,exportall"\
 -qsource \
 -g \
 -c \
 -I./chdsect

DLL_CPP_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink,exportall,showinc"\
 -qsource \
 -g \
 -c \
 -I./chdsect

DLL_BND_FLAGS=-W "l,lp64,dll,dynam=dll,xplink,map,list" \
 -g \
 -qsource

# -g generates debugging information
CPP_FLAGS=-W "c,lp64,langlvl(extended0x),dll,xplink,showinc"\
 -qsource \
 -g \
 -c

ASM_FLAGS=-mrent

all: libzut.so libzut.a libzds.so libzds.a libzusf.so libzusf.a libzcn.so libzcn.a libzjb.so libzjb.a zowex zowexx extattr test

#
# Utilities
#
zut.o: zut.cpp
	@echo 'Building zut.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

zutm.s: zutm.c
	@echo 'Building zutm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

zutm.o: zutm.s
	@echo 'Building zutm.s'
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

zutm31.s: zutm31.c
	@echo 'Building zutm31.s'
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

zutm31.o: zutm31.s
	@echo 'Building zutm31.o'
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

libzut.so: zut.o zutm.o zutm31.o
	@echo 'Building libzut.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libzut.a: zut.o zutm.o zutm31.o
	@echo 'Building libzut.a'
	ar -rv $@ $^

#
# Data sets
#
zdsm.s: zdsm.c
	@echo 'Building zdsm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

zdsm.o: zdsm.s
	@echo 'Building zdsm.o'
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

zds.o: zds.cpp
	@echo 'Building zds.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

libzds.so: zds.o zdsm.o libzut.a
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst
	@echo 'Building libzds.so'

libzds.a: zds.o zdsm.o libzut.a
	@echo 'Building libzds.a'
	ar -rv $@ $^

#
# Jobs
#
zjbm.s: zjbm.c
	@echo 'Building zjbm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

zjbm.o: zjbm.s
	@echo 'Building zjbm.o'
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

zjb.o: zjb.cpp
	@echo 'Building zjb.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

zjbm31.s: zjbm31.c
	@echo 'Building zjbm31.s'
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

zjbm31.o: zjbm31.s
	@echo 'Building zjbm31.o'
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

zssi31.s: zssi31.c
	@echo 'Building zssi31.s'
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

zssi31.o: zssi31.s
	@echo 'Building zssi31.o'
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

libzjb.so: zjb.o zjbm.o zssi31.o zjbm31.o libzds.a libzut.a
	@echo 'Building libzjb.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libzjb.a: zjb.o zjbm.o zssi31.o zjbm31.o libzds.a libzut.a
	@echo 'Building libzjb.a'
	ar -rv $@ $^

#
# Console
#
zcn.o: zcn.cpp
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

zcnm.s: zcnm.c
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

zcnm.o: zcnm.s
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

zcnm31.s: zcnm31.c
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

zcnm31.o: zcnm31.s
	$(ASM) $(ASM_FLAGS) -a=$*.asm.lst $(MACLIBS) -o $@ $^

libzcn.so: zcn.o zcnm.o zcnm31.o libzut.a
	@echo 'Building libzcn.so'
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst

libzcn.a: zcn.o zcnm.o zcnm31.o libzut.a
	@echo 'Building libzcn.a'
	ar -rv $@ $^

#
# USS data
#
zusf.o: zusf.cpp
	@echo 'Building zusf.o'
	$(CXX) $(DLL_CPP_FLAGS) -qlist=$*.cpp.lst -o $@ $^

libzusf.so: zusf.o libzut.a
	$(CXX) $(DLL_BND_FLAGS) -o $@ $^ > $*.bind.lst
	@echo 'Building libzusf.so'

libzusf.a: zusf.o libzut.a
	@echo 'Building libzusf.a'
	ar -rv $@ $^

#
# Test CLI
#
# zowex: zowex.o zcn.o zcnm.o zcnm31.o zut.o zutm.o zutm31.o zjb.o zjbm.o zssi31.o zjbm31.o zds.o
zowex: zowex.o libzcn.a libzut.a libzjb.a libzds.a libzusf.a
	@echo 'Building zowex'
	$(CXX) $(CPP_BND_FLAGS) -o $@ $^ > $*.bind.lst
	cp $@ ../golang/$(basename $@)
	@echo 'Copied zowex to golang/'

zowexx: zowex.o libzcn.a libzut.a libzjb.a libzds.a libzusf.a
	@echo 'Building zowexx'
	$(CXX) $(CPP_BND_FLAGS_AUTH) -o $@ $^ > $*.bind.lst
	cp $@ ../golang/$(basename $@)
	@echo 'Copied zowexx to golang/'


zowex.o: zowex.cpp
	@echo 'Building zowex.o'
	$(CXX) $(CPP_FLAGS) -qlist=$*.cpp.lst -c $^

test: test.sh
	chmod +x test.sh

#
# TODO(Kelosky): makefile for chdsect (must have ADATA in data set)
#
# ISPF: tso alloc da('DKELOSKY.TEMP.ADATA') DSORG(PO) SPACE(5,5) CYL LRECL(80) RECFM(F,b) NEW vol(tsu063)
# zowex: zowex test bpxwdyn "alloc da('DKELOSKY.TEMP.TEST2') DSORG(PO) SPACE(5,5) CYL LRECL(80) RECFM(F,b) NEW keep"
# z/OS UNIX: tso "alloc da('dkelosky.temp.adata') DSORG(PO) SPACE(5,5) CYL LRECL(80) RECFM(F,b) NEW vol(tsu063)"
# as -madata --gadata="//'DKELOSKY.TEMP.ADATA(IHAECB)'" ihaecb.s
#

#
# Misc targets
#
extattr:
	extattr +ap zowexx ../golang/zowexx

clean:
	rm -f *.o
	rm -f *.dbg
	rm -f *.so
	rm -f *.a
	rm -f *.u
	rm -f *.x
	rm -f *.lst
	rm -f CEEDUMP.*