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

# Common toolchain definitions for examples
.INCLUDE: ../../toolchain.mk
# WARNING: Since this file is meant to be included by other files, we need to use the relative path to the toolchain.mk file.

# Common directories for examples
MACLIBS= -I../../../asmmac $(MACLIBS_BASE)
MTL_HEADERS=$(MTL_HEADERS_BASE) -I../../chdsect -I../../../c

# Common flags for examples
C_FLAGS_COMMON=-I../../chdsect -I../../../c
CPP_FLAGS_COMMON=-I../../chdsect -I../../../c

# Build type handling for examples
.IF $(BuildType) == DEBUG
CPP_BND_FLAGS=$(CPP_BND_DEBUG_FLAGS) $(DEBUGGER_FLAGS)
C_FLAGS=$(C_FLAGS_BASE) $(C_FLAGS_COMMON) $(DEBUGGER_FLAGS)
CPP_FLAGS=$(CPP_FLAGS_BASE) $(CPP_FLAGS_COMMON) $(DEBUGGER_FLAGS)
MTL_FLAGS+=$(DEBUGGER_FLAGS) $(OTHER_C_FLAGS)
MTL_FLAGS64+=$(DEBUGGER_FLAGS) $(OTHER_C_FLAGS)
ASM_FLAGS+=--verbose
.ELIF $(BuildType) == RELEASE
CPP_BND_FLAGS=$(CPP_BND_BASE_FLAGS)
C_FLAGS=$(C_FLAGS_BASE) $(C_FLAGS_COMMON) $(RELEASE_FLAGS)
CPP_FLAGS=$(CPP_FLAGS_BASE) $(CPP_FLAGS_COMMON) $(RELEASE_FLAGS)
ASM_FLAGS+=--noverbose
MTL_FLAGS+=-g1
MTL_FLAGS64+=-g1
.ELSE
CPP_BND_FLAGS=$(CPP_BND_BASE_FLAGS)
C_FLAGS=$(C_FLAGS_BASE) $(C_FLAGS_COMMON) -g1
CPP_FLAGS=$(CPP_FLAGS_BASE) $(CPP_FLAGS_COMMON) -g1
ASM_FLAGS+=--noverbose
# Still investigating why we need `-g1` for the MetalC flags :'(
MTL_FLAGS+=-g1
MTL_FLAGS64+=-g1
.END

# Apply LOG_FLAGS if enabled
.IF DEFINED(LOG_FLAGS)
C_FLAGS+=$(LOG_FLAGS)
CPP_FLAGS+=$(LOG_FLAGS)
MTL_FLAGS+=$(LOG_FLAGS)
MTL_FLAGS64+=$(LOG_FLAGS)
.END
