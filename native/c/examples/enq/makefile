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

all: build-out run

run: build-out/runenq
	@echo 'Running ./build-out/runenq'
	./build-out/runenq

build-out:
	@echo 'Creating build-out'
	mkdir -p build-out

build-out/runenq.s: runenq.c
	@echo 'Building build-out/runenq.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/runenq.o: build-out/runenq.s
	@echo 'Building build-out/runenq.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/runenq: build-out/runenq.o
	@echo 'Building build-out/runenq'
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
