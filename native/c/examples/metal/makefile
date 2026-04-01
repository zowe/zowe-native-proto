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

.INCLUDE: ../examples_toolchain.mk

all: copy-mtl build-out run

run: build-out/demometal
	@echo 'Running ./build-out/demometal'
	./build-out/demometal

build-out:
	@echo 'Creating build-out'
	mkdir -p build-out

copy-mtl:
	cp ../../zutm.c zutm.c
	cp ../../ztime.h ztime.h

build-out/zutm.s: zutm.c
	@echo 'Building $(OUT_DIR)/zutm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/zutm.o: build-out/zutm.s
	@echo 'Building $(OUT_DIR)/zutm.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/demometal.s: demometal.c
	@echo 'Building build-out/demometal.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/demometal.o: build-out/demometal.s
	@echo 'Building build-out/demometal.s'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/demometal: build-out/demometal.o
	@echo 'Building build-out/demometal'
	$(CXX) $(CPP_BND_FLAGS) -Wl,-eMAIN -o $@ $^

clean:
	rm -f *.o
	rm -f *.dbg
	rm -f *.so
	rm -f *.a
	rm -f *.x
	rm -f *.lst
	rm -f CEEDUMP.*
	rm -rf build-out
