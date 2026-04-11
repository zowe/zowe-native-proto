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

.INCLUDE: ../plugins_toolchain.mk

all: $(OUT_DIR) run

run: $(OUT_DIR)/time_plugin.so
	@echo 'Running ./$(OUT_DIR)/time_plugin.so'
	mkdir -p ../../build-out/plugins
	cp ./$(OUT_DIR)/time_plugin.so ../../build-out/plugins/time_plugin.so
	cd ../../build-out && ./zowex plugins list
	cd ../../build-out && ./zowex time

$(OUT_DIR):
	@echo 'Creating $(OUT_DIR)'
	mkdir -p $(OUT_DIR)

$(OUT_DIR)/time_plugin.o: time_plugin.cpp
	@echo 'Building $(OUT_DIR)/time_plugin.o'
	$(CXX) $(CPP_FLAGS) -c -o $@ $^

$(OUT_DIR)/time_plugin.so: $(OUT_DIR)/time_plugin.o
	@echo 'Building $(OUT_DIR)/time_plugin.so'
	$(CXX) $(DLL_BND_BASE_FLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f *.dbg
	rm -f *.so
	rm -f *.a
	rm -f *.x
	rm -f *.lst
	rm -f CEEDUMP.*
	rm -f ../../build-out/plugins/time_plugin.so
	rm -rf $(OUT_DIR)
