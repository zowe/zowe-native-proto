*  This program and the accompanying materials are
*  made available under the terms of the Eclipse Public License v2.0
*  which accompanies this distribution, and is available at
*  https://www.eclipse.org/legal/epl-v20.html
*
*  SPDX-License-Identifier: EPL-2.0
*
*  Copyright Contributors to the Zowe Project.
ZAM24    RSECT ,
ZAM24    AMODE 31
ZAM24    RMODE 31                   Manually loaded in 24 bit storage
*
         YREGS ,
*
*        On entry: R1=DCB abend parameter list, R14=return address
*        Call ZAMDA31 (AMODE 31) with R1 preserved, then return
*        R2 is used to hold R14 across the call (BAKR preserves R2)
*
ENTRY24  DS    0H
         LARL  R15,DCBABND@
         L     R15,0(,R15)         -> ZAMDA31
         OILH  R15,X'8000'         Set AMODE 31 bit for BASSM
         BASSM 0,R15               Call ZAMDA31 in AMODE 31
         BR    R14                 Return to system
*
CONSTANT DS    0D
         LTORG ,
*
DCBABND@ DC    V(ZAMDEXIT)
*
*        Separate ENTRY to obtain length of this module
*
         ENTRY ZAM24Q
ZAM24Q   DS    0H
         LHI   R15,ZQM24LEN         = length of this module
         BR    R14                  return to caller
*
ZQM24LEN EQU *-ZAM24                Dynamically obtain length of module
*
         END   ,
