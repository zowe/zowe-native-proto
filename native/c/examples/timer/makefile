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

BIND=ld

all: build-out run

build-out:
	@echo 'Creating build-out'
	mkdir -p build-out

run: build-out/testtime
	@echo 'Running ./build-out/testtime'
	export _BPXK_JOBLOG=STDERR && ./build-out/testtime

build-out/testtime.s: testtime.c
	@echo 'Building build-out/testtime.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/testtime.o: build-out/testtime.s
	@echo 'Building build-out/testtime.s'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/testtime: build-out/testtime.o
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
