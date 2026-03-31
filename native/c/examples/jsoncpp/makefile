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

#
# Output directories
#
build-out:
	@echo 'Creating build-out'
	mkdir -p build-out

run: build-out/json
	@echo 'Running ./build-out/json'
	./build-out/json

copy-mtl:
	cp ../../zjsonm.c zjsonm.c

build-out/zjsonm.s: zjsonm.c
	@echo 'Building $(OUT_DIR)/zjsonm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/zjsonm.o: build-out/zjsonm.s
	@echo 'Building $(OUT_DIR)/zjsonm.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/json.o: json.cpp
	@echo 'Building $(OUT_DIR)/json.o'
	$(CXX) $(CPP_FLAGS) -c $^ -o $@

build-out/json: build-out/json.o build-out/zjsonm.o
	@echo 'Building json'
	$(CXX) $(CPP_BND_FLAGS) -o $@ $^ > $*.bind.lst

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
