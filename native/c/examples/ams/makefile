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

run: build-out/ams_driver
	@echo 'Running ./build-out/ams_driver'
	./build-out/ams_driver

copy-mtl:
	cp ../../zdsm.c zdsm.c
	cp ../../zam.c zam.c
	cp ../../zutm.c zutm.c
	cp ../../zutm31.c zutm31.c

build-out/ams_driver.o: ams_driver.cpp
	@echo 'Building $(OUT_DIR)/ams_driver.o'
	$(CXX) $(CPP_FLAGS) -c $^ -o $@

build-out/zds.o: ../../zds.cpp
	@echo 'Building $(OUT_DIR)/zds.o'
	$(CXX) $(CPP_FLAGS) -c $^ -o $@

build-out/zut.o: ../../zut.cpp
	@echo 'Building $(OUT_DIR)/zut.o'
	$(CXX) $(CPP_FLAGS) -c $^ -o $@

build-out/zdsm.s: zdsm.c
	@echo 'Building $(OUT_DIR)/zdsm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/zdsm.o: build-out/zdsm.s
	@echo 'Building $(OUT_DIR)/zdsm.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/zam.s: zam.c
	@echo 'Building $(OUT_DIR)/zam.s'
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/zam.o: build-out/zam.s
	@echo 'Building $(OUT_DIR)/zam.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/zutm.s: zutm.c
	@echo 'Building $(OUT_DIR)/zutm.s'
	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/zutm.o: build-out/zutm.s
	@echo 'Building $(OUT_DIR)/zutm.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/zutm31.s: zutm31.c
	@echo 'Building $(OUT_DIR)/zutm31.s'
	$(CC) $(MTL_FLAGS) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

build-out/zutm31.o: build-out/zutm31.s
	@echo 'Building $(OUT_DIR)/zutm31.o'
	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

# ams: build-out/ams

# build-out/ams.s: ams.c
# 	@echo 'Building build-out/ams.s'
# 	$(CC) $(MTL_FLAGS64) -qlist=$*.mtl.lst $(MTL_HEADERS) -o $@ $^

# build-out/ams.o: build-out/ams.s
# 	@echo 'Building build-out/ams.s'
# 	$(ASM) $(ASM_FLAGS) -a=$*.s.lst $(MACLIBS) -o $@ $^

build-out/ams_driver: build-out/zds.o build-out/zdsm.o build-out/zut.o build-out/zam.o build-out/zutm.o build-out/zutm31.o build-out/ams_driver.o
	@echo 'Building ams_driver'
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
