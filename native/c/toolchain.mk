#
#  This program and the accompanying materials are
#  made available under the terms of the Eclipse Public License v2.0
#  which accompanies this distribution, and is available at
#  https://www.eclipse.org/legal/epl-v20.html
#
#  SPDX-License-Identifier: EPL-2.0
#
#  Shared toolchain definitions for Zowe native components.
#

XLC_FLAGS=_CC_ACCEPTABLE_RC=0 _C89_ACCEPTABLE_RC=0 _CXX_ACCEPTABLE_RC=0
CXX=ibm-clang++64
CXXLANG=ibm-clang++64
CC=$(XLC_FLAGS) xlc
ASM=as

CPP_BND_BASE_FLAGS=-Wl,-bMAP $(LDFLAGS)
CPP_BND_BASE_FLAGS_AUTH=-Wl,-bMAP -Wl,-bAC=1 $(LDFLAGS)
CPP_BND_DEBUG_FLAGS=-Wl,-bMAP -Wl,-bLIST $(LDFLAGS)
CPP_BND_DEBUG_FLAGS_AUTH=-Wl,-bMAP -Wl,-bLIST -Wl,-bAC=1 $(LDFLAGS)

DLL_BND_BASE_FLAGS=-shared -Wl,-bMAP $(LDFLAGS)
DLL_BND_DEBUG_FLAGS=-shared -Wl,-bMAP -Wl,-bLIST $(LDFLAGS)

#
# Metal C compilation options
#
MTL_BASE_OPTS=metal,\
 langlvl(extended),\
 sscom,\
 nolongname,\
 inline,\
 genasm,\
 csect,\
 nose,\
 warn64,\
 optimize(2)
MTL_LIST_OPTS=,inlrpt,list,aggregate

MTL_OPTS=$(MTL_BASE_OPTS)
.IF $(BuildType) == DEBUG
MTL_OPTS+=$(MTL_LIST_OPTS)
.END
MTL_OPTS64=$(MTL_OPTS),lp64
MTL_FLAGS=-S -W "c,$(MTL_OPTS)"
MTL_FLAGS64=-S -W "c,$(MTL_OPTS64)"

#
# Assembly macros and headers
#
MACLIBS_BASE= -ISYS1.MACLIB \
 -ISYS1.MODGEN \
 -ICBC.SCCNSAM

MTL_HEADERS_BASE=-I/usr/include/metal \
 -I/usr/include

ASM_FLAGS=-mRENT

#
# Compilation flags
#
C_FLAGS_BASE=-fvisibility=default -c
DLL_CPP_FLAGS_BASE=-fvisibility=default -c -std=gnu++11 -fno-aligned-allocation -D_EXT -D_OPEN_SYS_FILE_EXT=1
CPP_FLAGS_BASE=-fvisibility=default -c -std=gnu++11 -fno-aligned-allocation -D_EXT -D_OPEN_SYS_FILE_EXT=1
CXXLANG_FLAGS_BASE=-c -std=gnu++11 -fno-aligned-allocation -D_EXT -D_OPEN_SYS_FILE_EXT=1
SWIG_FLAGS_BASE=-DSWIG -c -std=gnu++11 -fno-aligned-allocation -D_EXT -D_OPEN_SYS_FILE_EXT=1

#
# Logging support
#
.IF $(ZLOG_ENABLE) == 1
LOG_FLAGS=-DZLOG_ENABLE
.END

#
# Debug/Production build handling
#
.IF $(BuildType) == DEBUG
LOG_FLAGS=-DZLOG_ENABLE
DEBUGGER_FLAGS=-g
OTHER_C_FLAGS=-H -dM
.END
