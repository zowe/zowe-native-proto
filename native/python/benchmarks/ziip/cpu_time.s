* SPDX-License-Identifier: EPL-2.0
* Copyright Contributors to the Zowe Project.
         SYSSTATE AMODE64=YES
ZBCLK    CELQPRLG BASEREG=8,PARMREG=9
ZBCLK    ALIAS C'znp_timeused'
* PARMREG addresses the saved argument list, not its pointer argument.
         LG    9,0(9)
* TIME_ON_CP requires a below-bar F4SA, independent of the XPLINK DSA.
         LA    13,32(9)
         MVC   4(4,13),=C'F4SA'
         LLGF  14,16
         TM    CVTOSLV8-CVT(14),CVTECT1
         JZ    ZBNOSUP
         TM    FLCFACL3-PSA,FLCFECT
         JZ    ZBNOSUP
         TIMEUSED STORADR=(9),ECT=YES,TIME_ON_CP=YES,                  X
               OFFLOAD_TIME=YES,OFFLOAD_ON_CP=YES
         LGR   3,15
         J     ZBDONE
ZBNOSUP  LHI   3,16
ZBDONE   DS    0H
         CELQEPLG
         LTORG
ZBDSA    DSECT
         CEEDSA SECTYPE=XPLINK
         CVT   DSECT=YES
         IHAECVT
         IHAPSA
         END
