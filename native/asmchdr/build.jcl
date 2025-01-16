//*
//*  This program and the accompanying materials are
//*  made available under the terms of the Eclipse Public License v2.0
//*  which accompanies this distribution, and is available at
//*  https://www.eclipse.org/legal/epl-v20.html
//*
//*  SPDX-License-Identifier: EPL-2.0
//*
//*  Copyright Contributors to the Zowe Project.
//*
// JOB
// SET HLQ=IBMUSER
// SET SRC=CVT
// SET DPARM='PPCOND,EQUATE(DEF),BITF0XL,HDRSKIP,UNIQ,LP64,LEGACY'
//*
//         IF (RC = 0) THEN
//ASM4 EXEC PGM=ASMA90
//ASMAOPT  DD  *
ADATA
RENT
MACHINE(ZSERIES-5)
LIST(133)
/*
//SYSADATA DD  DISP=SHR,DSN=&HLQ..ADATA(&SRC.)
//SYSLIB   DD  DISP=SHR,DSN=&HLQ..ASMMAC
//         DD  DISP=SHR,DSN=SYS1.MACLIB
//         DD  DISP=SHR,DSN=SYS1.MODGEN
//         DD  DISP=SHR,DSN=ASMA.SASMMAC2
//         DD  DISP=SHR,DSN=CBC.SCCNSAM
//         DD  DISP=SHR,DSN=TCPIP.AEZAMAC1
//SYSPRINT DD  SYSOUT=*
//SYSIN    DD  DISP=SHR,DSN=&HLQ..ASMPGM(&SRC.)
//SYSLIN   DD  DISP=SHR,DSN=&HLQ..OBJLIB(&SRC.)
//         ENDIF
//         IF (RC = 0) THEN
//DSECT4 EXEC PGM=CCNEDSCT,
//         PARM='&DPARM,SECT(ALL)',
//         MEMLIMIT=256M
//STEPLIB  DD  DISP=SHR,DSN=CEE.SCEERUN2
//         DD  DISP=SHR,DSN=CBC.SCCNCMP
//         DD  DISP=SHR,DSN=CEE.SCEERUN
//SYSADATA DD  DISP=SHR,DSN=&HLQ..ADATA(&SRC.)
//EDCDSECT DD  DISP=SHR,DSN=&HLQ..CHDR(&SRC.)
//SYSPRINT DD SYSOUT=*
//SYSOUT   DD SYSOUT=*
//         ENDIF
