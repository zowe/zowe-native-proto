*  This program and the accompanying materials are
*  made available under the terms of the Eclipse Public License v2.0
*  which accompanies this distribution, and is available at
*  https://www.eclipse.org/legal/epl-v20.html
*
*  SPDX-License-Identifier: EPL-2.0
*
*  Copyright Contributors to the Zowe Project.
TESTLOAD RSECT ,
TESTLOAD AMODE 31
TESTLOAD RMODE ANY
*
         SYSSTATE ARCHLVL=2

         STM   14,12,12(13)        Save registers
         LR    2,1                 Save input parameters
*
         LARL  12,CONSTANT
         USING CONSTANT,12
*
*        IEBFCOPY needs 24 bit save area
*
         STORAGE OBTAIN,                                               x
               LENGTH=WRKLEN,                                          x
               LOC=(24,64),                                            x
               COND=NO
*
         LR    10,1                 -> new storage
         USING WRK,10
*
         LA    0,WRK                -> new storage
         LHI   1,WRKLEN
         SLR   15,15
         MVCL  0,14
*
         LA    3,WRKSAVE
CALR     USING SAVER,13
OUR      USING SAVER,3
         ST    3,CALR.SAVNEXT
         ST    13,OUR.SAVPREV
*
         DROP  OUR,CALR
         LR    13,3
         USING SAVER,13
*
         TAM   ,
         BRC   8,AMODE24
         BRC   4,AMODE31
         BRC   1,AMODE64
         EXRL  0,*                    should never get here
AMODE24  DS    0H
         WTO   'AMODE24'
         J      DOLOAD
AMODE31  DS    0H
         WTO   'AMODE31'
         J      DOLOAD
AMODE64  DS    0H
         WTO   'AMODE64'
         J      DOLOAD
*
DOLOAD   DS    0H
*
         LOAD  EPLOC==CL8'IEBCOPY',                                    x
               PLISTVER=MAX,                                           x
               ERRET=FAILED
*
         LR    2,0                     -> ep
*
         WTO   'LOADED'
*
         L     1,=F'1234'
*
         LA    15,0(,2)
         BASR  14,15
*
         WTO   'EXITED'
*
         J      CONTINUE
*
FAILED   DS    0H
         WTO   'FAILED'
         J      EXIT
*
CONTINUE DS    0H
         WTO   'CONTINUE'
*
EXIT     DS    0H
         L     13,SAVPREV
*
         LA    1,WRK
         STORAGE RELEASE,                                              x
               LENGTH=WRKLEN,                                          x
               ADDR=(1)
*
         LM    14,12,12(13)
         LHI   15,0
         BR    14
*
CONSTANT DS    0D
         LTORG ,
*
WRK      DSECT ,
WRKSAVE  DS    XL72
WRKLEN   EQU   *-WRK
         IHASAVER ,
         END
