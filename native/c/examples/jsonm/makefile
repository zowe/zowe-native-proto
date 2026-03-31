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

.INCLUDE: ../toolchain.mk

all: build-out build-out/json

json: build-out/json

build-out:
	@echo 'Creating build-out'
	mkdir -p build-out

build-out/json.s: json.c
	@echo 'Building build-out/json.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/json.o: build-out/json.s
	@echo 'Building build-out/json.s'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/json: build-out/json.o
	@echo 'Building build-out/json'
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
