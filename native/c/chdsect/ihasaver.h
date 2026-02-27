#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif
 
#ifndef __saver__
#define __saver__
 
struct saver {
  void * __ptr32 savpli;   /* USED BY PL/I LANG. PRGM   */
  void * __ptr32 savprev;  /* ADDR OF PREVIOUS SAVEAREA */
  void * __ptr32 savnext;  /* ADDR OF NEXT SAVE AREA    */
  void * __ptr32 savgrs14; /* REGISTER 14               */
  void * __ptr32 savgrs15; /* REGISTER 15               */
  void * __ptr32 savgrs0;  /* REGISTER 0                */
  void * __ptr32 savgrs1;  /* REGISTER 1                */
  void * __ptr32 savgrs2;  /* REGISTER 2                */
  void * __ptr32 savgrs3;  /* REGISTER 3                */
  void * __ptr32 savgrs4;  /* REGISTER 4                */
  void * __ptr32 savgrs5;  /* REGISTER 5                */
  void * __ptr32 savgrs6;  /* REGISTER 6                */
  void * __ptr32 savgrs7;  /* REGISTER 7                */
  void * __ptr32 savgrs8;  /* REGISTER 8                */
  void * __ptr32 savgrs9;  /* REGISTER 9                */
  void * __ptr32 savgrs10; /* REGISTER 10               */
  void * __ptr32 savgrs11; /* REGISTER 11               */
  void * __ptr32 savgrs12; /* REGISTER 12               */
  };
 
/* Values for field "savgrs12" */
#define saver___len 0x48
 
#endif
 
#ifndef __savf4sa__
#define __savf4sa__
 
struct savf4sa {
  void * __ptr32 savf4salang;       /* USED BY LANGUAGES         */
  unsigned char  savf4said[4];      /* 'F4SA'                    */
  unsigned char  savf4sag64rs14[8]; /* REGISTER 14               */
  unsigned char  savf4sag64rs15[8]; /* REGISTER 15               */
  unsigned char  savf4sag64rs0[8];  /* REGISTER 0                */
  unsigned char  savf4sag64rs1[8];  /* REGISTER 1                */
  unsigned char  savf4sag64rs2[8];  /* REGISTER 2                */
  unsigned char  savf4sag64rs3[8];  /* REGISTER 3                */
  unsigned char  savf4sag64rs4[8];  /* REGISTER 4                */
  unsigned char  savf4sag64rs5[8];  /* REGISTER 5                */
  unsigned char  savf4sag64rs6[8];  /* REGISTER 6                */
  unsigned char  savf4sag64rs7[8];  /* REGISTER 7                */
  unsigned char  savf4sag64rs8[8];  /* REGISTER 8                */
  unsigned char  savf4sag64rs9[8];  /* REGISTER 9                */
  unsigned char  savf4sag64rs10[8]; /* REGISTER 10               */
  unsigned char  savf4sag64rs11[8]; /* REGISTER 11               */
  unsigned char  savf4sag64rs12[8]; /* REGISTER 12               */
  unsigned char  savf4saprev[8];    /* ADDR OF PREVIOUS SAVEAREA */
  unsigned char  savf4sanext[8];    /* ADDR OF NEXT SAVE AREA    */
  };
 
/* Values for field "savf4sanext" */
#define savf4said___value 0xC6F4E2C1
#define savf4sa___len     0x90
 
#endif
 
#ifndef __savf5sa__
#define __savf5sa__
 
struct savf5sa {
  void * __ptr32 savf5salang;       /* USED BY LANGUAGES         */
  unsigned char  savf5said[4];      /* 'F5SA'                    */
  unsigned char  savf5sag64rs14[8]; /* REGISTER 14               */
  unsigned char  savf5sag64rs15[8]; /* REGISTER 15               */
  unsigned char  savf5sag64rs0[8];  /* REGISTER 0                */
  unsigned char  savf5sag64rs1[8];  /* REGISTER 1                */
  unsigned char  savf5sag64rs2[8];  /* REGISTER 2                */
  unsigned char  savf5sag64rs3[8];  /* REGISTER 3                */
  unsigned char  savf5sag64rs4[8];  /* REGISTER 4                */
  unsigned char  savf5sag64rs5[8];  /* REGISTER 5                */
  unsigned char  savf5sag64rs6[8];  /* REGISTER 6                */
  unsigned char  savf5sag64rs7[8];  /* REGISTER 7                */
  unsigned char  savf5sag64rs8[8];  /* REGISTER 8                */
  unsigned char  savf5sag64rs9[8];  /* REGISTER 9                */
  unsigned char  savf5sag64rs10[8]; /* REGISTER 10               */
  unsigned char  savf5sag64rs11[8]; /* REGISTER 11               */
  unsigned char  savf5sag64rs12[8]; /* REGISTER 12               */
  unsigned char  savf5saprev[8];    /* ADDR OF PREVIOUS SAVEAREA */
  unsigned char  savf5sanext[8];    /* ADDR OF NEXT SAVE AREA    */
  void * __ptr32 savf5sag64hs0;     /* High half of caller's R0  */
  void * __ptr32 savf5sag64hs1;     /* High half of caller's R1  */
  void * __ptr32 savf5sag64hs2;     /* High half of caller's R2  */
  void * __ptr32 savf5sag64hs3;     /* High half of caller's R3  */
  void * __ptr32 savf5sag64hs4;     /* High half of caller's R4  */
  void * __ptr32 savf5sag64hs5;     /* High half of caller's R5  */
  void * __ptr32 savf5sag64hs6;     /* High half of caller's R6  */
  void * __ptr32 savf5sag64hs7;     /* High half of caller's R7  */
  void * __ptr32 savf5sag64hs8;     /* High half of caller's R8  */
  void * __ptr32 savf5sag64hs9;     /* High half of caller's R9  */
  void * __ptr32 savf5sag64hs10;    /* High half of caller's R10 */
  void * __ptr32 savf5sag64hs11;    /* High half of caller's R11 */
  void * __ptr32 savf5sag64hs12;    /* High half of caller's R12 */
  void * __ptr32 savf5sag64hs13;    /* High half of caller's R13 */
  void * __ptr32 savf5sag64hs14;    /* High half of caller's R14 */
  void * __ptr32 savf5sag64hs15;    /* High half of caller's R15 */
  unsigned char  _filler1[8];       /* Undefined                 */
  };
 
/* Values for field "_filler1" */
#define savf5said___value 0xC6F5E2C1
#define savf5sa___len     0xD8
 
#endif
 
#ifndef __savf7sa__
#define __savf7sa__
 
struct savf7sa {
  void * __ptr32 savf7salang;       /* USED BY LANGUAGES             */
  unsigned char  savf7said[4];      /* 'F7SA'                        */
  unsigned char  savf7sag64rs14[8]; /* REGISTER 14                   */
  unsigned char  savf7sag64rs15[8]; /* REGISTER 15                   */
  unsigned char  savf7sag64rs0[8];  /* REGISTER 0                    */
  unsigned char  savf7sag64rs1[8];  /* REGISTER 1                    */
  unsigned char  savf7sag64rs2[8];  /* REGISTER 2                    */
  unsigned char  savf7sag64rs3[8];  /* REGISTER 3                    */
  unsigned char  savf7sag64rs4[8];  /* REGISTER 4                    */
  unsigned char  savf7sag64rs5[8];  /* REGISTER 5                    */
  unsigned char  savf7sag64rs6[8];  /* REGISTER 6                    */
  unsigned char  savf7sag64rs7[8];  /* REGISTER 7                    */
  unsigned char  savf7sag64rs8[8];  /* REGISTER 8                    */
  unsigned char  savf7sag64rs9[8];  /* REGISTER 9                    */
  unsigned char  savf7sag64rs10[8]; /* REGISTER 10                   */
  unsigned char  savf7sag64rs11[8]; /* REGISTER 11                   */
  unsigned char  savf7sag64rs12[8]; /* REGISTER 12                   */
  unsigned char  savf7saprev[8];    /* ADDR OF PREVIOUS SAVEAREA     */
  unsigned char  savf7sanext[8];    /* ADDR OF NEXT SAVE AREA        */
  int            savf7saar14;       /* AR 14                         */
  int            savf7saar15;       /* AR 15                         */
  int            savf7saar0;        /* AR 0                          */
  int            savf7saar1;        /* AR 1                          */
  int            savf7saar2;        /* AR 2                          */
  int            savf7saar3;        /* AR 3                          */
  int            savf7saar4;        /* AR 4                          */
  int            savf7saar5;        /* AR 5                          */
  int            savf7saar6;        /* AR 6                          */
  int            savf7saar7;        /* AR 7                          */
  int            savf7saar8;        /* AR 8                          */
  int            savf7saar9;        /* AR 9                          */
  int            savf7saar10;       /* AR 10                         */
  int            savf7saar11;       /* AR 11                         */
  int            savf7saar12;       /* AR 12                         */
  int            savf7saar13;       /* ALET of previous save area or */
  int            savf7saasc;        /* ASC mode of caller            */
  unsigned char  _filler1[4];       /* Undefined                     */
  };
 
/* Values for field "_filler1" */
#define savf7said___value 0xC6F7E2C1
#define savf7sa___len     0xD8
 
#endif
 
#ifndef __savf8sa__
#define __savf8sa__
 
struct savf8sa {
  void * __ptr32 savf8salang;       /* USED BY LANGUAGES             */
  unsigned char  savf8said[4];      /* 'F8SA'                        */
  unsigned char  savf8sag64rs14[8]; /* REGISTER 14                   */
  unsigned char  savf8sag64rs15[8]; /* REGISTER 15                   */
  unsigned char  savf8sag64rs0[8];  /* REGISTER 0                    */
  unsigned char  savf8sag64rs1[8];  /* REGISTER 1                    */
  unsigned char  savf8sag64rs2[8];  /* REGISTER 2                    */
  unsigned char  savf8sag64rs3[8];  /* REGISTER 3                    */
  unsigned char  savf8sag64rs4[8];  /* REGISTER 4                    */
  unsigned char  savf8sag64rs5[8];  /* REGISTER 5                    */
  unsigned char  savf8sag64rs6[8];  /* REGISTER 6                    */
  unsigned char  savf8sag64rs7[8];  /* REGISTER 7                    */
  unsigned char  savf8sag64rs8[8];  /* REGISTER 8                    */
  unsigned char  savf8sag64rs9[8];  /* REGISTER 9                    */
  unsigned char  savf8sag64rs10[8]; /* REGISTER 10                   */
  unsigned char  savf8sag64rs11[8]; /* REGISTER 11                   */
  unsigned char  savf8sag64rs12[8]; /* REGISTER 12                   */
  unsigned char  savf8saprev[8];    /* ADDR OF PREVIOUS SAVEAREA     */
  unsigned char  savf8sanext[8];    /* ADDR OF NEXT SAVE AREA        */
  int            savf8saar14;       /* AR 14                         */
  int            savf8saar15;       /* AR 15                         */
  int            savf8saar0;        /* AR 0                          */
  int            savf8saar1;        /* AR 1                          */
  int            savf8saar2;        /* AR 2                          */
  int            savf8saar3;        /* AR 3                          */
  int            savf8saar4;        /* AR 4                          */
  int            savf8saar5;        /* AR 5                          */
  int            savf8saar6;        /* AR 6                          */
  int            savf8saar7;        /* AR 7                          */
  int            savf8saar8;        /* AR 8                          */
  int            savf8saar9;        /* AR 9                          */
  int            savf8saar10;       /* AR 10                         */
  int            savf8saar11;       /* AR 11                         */
  int            savf8saar12;       /* AR 12                         */
  int            savf8saar13;       /* ALET of previous save area or */
  int            savf8saasc;        /* ASC mode of caller            */
  unsigned char  _filler1[4];       /* Undefined                     */
  void * __ptr32 savf8sag64hs0;     /* High half of caller's R0      */
  void * __ptr32 savf8sag64hs1;     /* High half of caller's R1      */
  void * __ptr32 savf8sag64hs2;     /* High half of caller's R2      */
  void * __ptr32 savf8sag64hs3;     /* High half of caller's R3      */
  void * __ptr32 savf8sag64hs4;     /* High half of caller's R4      */
  void * __ptr32 savf8sag64hs5;     /* High half of caller's R5      */
  void * __ptr32 savf8sag64hs6;     /* High half of caller's R6      */
  void * __ptr32 savf8sag64hs7;     /* High half of caller's R7      */
  void * __ptr32 savf8sag64hs8;     /* High half of caller's R8      */
  void * __ptr32 savf8sag64hs9;     /* High half of caller's R9      */
  void * __ptr32 savf8sag64hs10;    /* High half of caller's R10     */
  void * __ptr32 savf8sag64hs11;    /* High half of caller's R11     */
  void * __ptr32 savf8sag64hs12;    /* High half of caller's R12     */
  void * __ptr32 savf8sag64hs13;    /* High half of caller's R13     */
  void * __ptr32 savf8sag64hs14;    /* High half of caller's R14     */
  void * __ptr32 savf8sag64hs15;    /* High half of caller's R15     */
  unsigned char  _filler2[8];       /* Undefined                     */
  };
 
/* Values for field "_filler2" */
#define savf8said___value 0xC6F8E2C1
#define savf8sa___len     0x120
 
#endif
 
#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
