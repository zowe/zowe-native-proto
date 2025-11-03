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
