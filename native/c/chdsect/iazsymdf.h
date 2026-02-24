#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif

#ifndef __jsymparm__
#define __jsymparm__

struct jsymparm
{
  unsigned char jsymeye[4]; /* I.Eye catcher                       */
  unsigned short jsymlng;   /* I.Length of parameter list          */
  unsigned short jsymvrm;   /* I.Parameter ver/mod:                */
  unsigned short jsymsvrm;  /* O.Service ver/mod                   */
  unsigned char jsymrqop;   /* I.Requested operation:              */
  unsigned char jsymlvl;    /* I.Symbol options:                   */
  void *__ptr32 jsymisyt;   /* I. Pointer to an input symbol table */
  void *__ptr32 jsymsnma;   /* IS*. Pointer to selection list      */
  int jsymsnmn;             /* IS.  Number of elements in          */
  int jsymsnml;             /* IS.  Length of each element in      */
  void *__ptr32 jsymouta;   /* I. Pointer to caller provided       */
  int jsymouts;             /* I. Size of caller provided          */
  int jsymretn;             /* O. Service return code              */
  int jsymreas;             /* O. Service reason code              */
  int jsymsrcm;             /* O. Recommended size of the output   */
  void *__ptr32 jsymerad;   /* O. If service returns an error this */
  int _filler1[9];          /* Reserved                            */
};

/* Values for field "jsymrqop" */
#define jsymcrt 4   /* create symbols with values       */
#define jsymupdt 8  /* update symbol values             */
#define jsymdele 12 /* delete symbols                   */
#define jsymextr 16 /* extract values for symbols       */
#define jsymclr 20  /* clear all symbols                */

/* Values for field "jsymlvl" */
#define jsymlvlt 0x80 /* access symbols at the task level */
#define jsymlvlj 0x40 /* access symbols at the job step   */
#define jsymlvnj 0x20 /* do not access JCL symbols        */
#define jsymlvud 0x10 /* CREATE updates duplicates        */
#define jsymlvjc 0x08 /* check JCL constraints            */

/* Values for field "_filler1" */
#define jsymsze1 0x58 /* Version 1 length                 */
#define jsymsize 0x58 /* Current version length           */

#endif

#ifndef __jsytable__
#define __jsytable__

struct jsytable
{
  unsigned char jsyteye[4];  /* Eyecatcher                     */
  int jsytlen;               /* Total size of the table        */
  unsigned char jsytver;     /* Version of the table           */
  unsigned char _filler1[3]; /* Reserved                       */
  int jsytent1;              /* Offset from the beginning of   */
  int jsytentn;              /* Number of entries in the table */
  int jsytents;              /* Size of each entry             */
  int _filler2[2];           /* Reserved                       */
};

/* Values for field "jsytver" */
#define jsytver1 1 /* Version 1                */

/* Values for field "_filler2" */
#define jsytsiz1 0x20 /* Size of version 1        */
#define jsytsize 0x20 /* Size of the table header */

#endif

#ifndef __jsyentry__
#define __jsyentry__

struct jsyentry
{
  unsigned char jsyename[16]; /* Symbol name                        */
  int jsyevalo;               /* Offset from the beginning of table */
  unsigned short jsyevals;    /* Size of the symbol value           */
  unsigned char _filler1[2];  /* Reserved                           */
};

/* Values for field "_filler1" */
#define jsyesiz1 0x18 /* Size of the table entry */
#define jsyesize 0x18 /* Size of the table entry */

#endif

#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
