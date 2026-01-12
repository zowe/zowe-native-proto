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
CXX=$(XLC_FLAGS) xlc++
CXXLANG=xlclang++
CC=$(XLC_FLAGS) xlc
ASM=as

CPP_BND_BASE_FLAGS=-W "l,lp64,xplink,map"
CPP_BND_BASE_FLAGS_AUTH=-W "l,lp64,xplink,map,ac=1"
CPP_BND_DEBUG_FLAGS=-W "l,lp64,xplink,map,list"
CPP_BND_DEBUG_FLAGS_AUTH=-W "l,lp64,xplink,map,list,ac=1"

DLL_BND_BASE_FLAGS=-W "l,lp64,dll,dynam=dll,xplink,map"
DLL_BND_DEBUG_FLAGS=-W "l,lp64,dll,dynam=dll,xplink,map,list"

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
C_FLAGS_BASE=-W "c,lp64,langlvl(extended),xplink,exportall" -c
DLL_CPP_FLAGS_BASE=-W "c,lp64,langlvl(extended0x),dll,xplink,exportall" -c
CPP_FLAGS_BASE=-W "c,lp64,langlvl(extended0x),dll,xplink" -c -D__IBMCPP_TR1__=1
CXXLANG_FLAGS_BASE=-W "c,lp64" -c
SWIG_FLAGS_BASE=-W "c,lp64,define(SWIG)" -c -DSWIG

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
DEBUGGER_FLAGS=-qSOURCE -g9
OTHER_C_FLAGS=-qSHOWINC -qSHOWMACROS
.END
