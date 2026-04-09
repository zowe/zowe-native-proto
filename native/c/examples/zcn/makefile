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

all: build-out extattr run 

#
# Output directories
#
build-out:
	@echo 'Creating build-out'
	mkdir -p build-out

run: build-out/demozcnx
	@echo 'Running ./build-out/demozcnx'
	export _BPXK_JOBLOG=STDERR && ./build-out/demozcnx

build-out/demozcn.o: demozcn.cpp
	@echo 'Building $(OUT_DIR)/demozcn.o'
	cp ../../build-out/*.a build-out
	$(CXX) $(CPP_FLAGS) -c $^ -o $@

build-out/demozcnx: build-out/demozcn.o build-out/libzcn.a build-out/libzut.a
	@echo 'Building demozcnx'
	$(CXX) $(CPP_BND_FLAGS_AUTH) -o $@ $^ > $*.bind.lst

extattr:
	extattr +ap build-out/demozcnx

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