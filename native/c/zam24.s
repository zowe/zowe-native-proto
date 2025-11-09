*  This program and the accompanying materials are
*  made available under the terms of the Eclipse Public License v2.0
*  which accompanies this distribution, and is available at
*  https://www.eclipse.org/legal/epl-v20.html
*
*  SPDX-License-Identifier: EPL-2.0
*
*  Copyright Contributors to the Zowe Project.
ZAM24    RSECT ,
ZAM24    AMODE 24
ZAM24    RMODE 24                   Manually loaded in 24 bit storage
*
RTNCD00  EQU   0
*
         YREGS ,
*
*        TODO(Kelosky): find address of real DCBABEND
*
         USING ZAM24,R15
*
         EXRL  0,*
         L     R1,DCBABND@         -> ZAMDA31
         LHI   R15,RTNCD00
         BR    R14                  return to caller
*
CONSTANT DS    0D
         LTORG ,
*
DCBABND@ DC    V(ZAMDA31)
*
*        Separate ENTRY to obtain length of this module
*
         ENTRY ZAM24Q
ZAM24Q   DS    0H
         LHI   R15,ZQM24LEN         = length of this module
         BR    R14                  return to caller
*
ZQM24LEN EQU *-ZAM24                Dyamically obtain len of module
*
         END   ,
