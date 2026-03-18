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
*        DCB ABEND exit routine
*        On entry: R1 = DCB ABEND parm list addr (low 3 bytes)
*                  R14 = return address
*        Calls ZAMDA31(parms) to handle the abend
*
         YREGS ,
*
         USING ZAM24,R15
*
         STM   R14,R12,12(R13)     Save caller's registers
         LR    R12,R15             Establish base
         DROP  R15
         USING ZAM24,R12
*
         LA    R1,0(,R1)           Mask R1 to get parm list address
         L     R15,DCBABND@        -> ZAMDA31
         BALR  R14,R15             Call ZAMDA31(parms)
*
         LM    R14,R12,12(R13)     Restore caller's registers
         BR    R14                 Return to system
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
         LHI   R15,ZQM24LEN       = length of this module
         BR    R14                 return to caller
*
ZQM24LEN EQU *-ZAM24              Dynamically obtain length of module
*
         END   ,
