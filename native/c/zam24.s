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
ZAM24    RMODE 31                   Resolve 31-bit address
*
         YREGS ,
*
*        On entry: R1=DCB abend parameter list, R14=return address
*        Call ZAMDEXIT with valid C argument list, then return
*
ENTRY24  DS    0H
*        Save system return address and R1 in local storage
         LARL  R15,MY_R14
         ST    R14,0(,R15)
         LARL  R15,MY_R1
         ST    R1,0(,R15)
*        Prepare R1 as a standard C parameter list
         LR    R1,R15
*        Branch to C code
         LARL  R15,DCBABND@
         L     R15,0(,R15)         -> ZAMDEXIT
         OILH  R15,X'8000'         Set AMODE 31 bit for BASSM
         BASSM 14,R15              Call ZAMDEXIT in AMODE 31
*        ZAMDEXIT returns here. R15 contains the return code.
         LARL  R1,MY_R1
         L     R1,0(,R1)           Restore original system R1
         STC   R15,3(,R1)          Store RC into option_mask (byte 3)
*        Restore R14 and return to system
         LARL  R14,MY_R14
         L     R14,0(,R14)
         BR    R14                 Return to system
*
CONSTANT DS    0D
         LTORG ,
*
DCBABND@ DC    V(ZAMDEXIT)
MY_R1    DS    A
MY_R14   DS    A
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
