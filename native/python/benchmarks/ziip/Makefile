# SPDX-License-Identifier: EPL-2.0
# Copyright Contributors to the Zowe Project.

CC = ibm-clang64
AS = as

all: cpu_time.so

cpu_time_asm.o: cpu_time.s
	$(AS) -mgoff -mRENT -ISYS1.MACLIB -ISYS1.MODGEN -ICEE.SCEEMAC -o $@ $<

cpu_time.o: cpu_time.c
	$(CC) -c -fvisibility=default -o $@ $<

cpu_time.so: cpu_time.o cpu_time_asm.o
	$(CC) -shared -o $@ cpu_time.o cpu_time_asm.o

.PHONY: all
