#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif

#ifndef __dsinf__
#define __dsinf__

struct dsinf
{
  unsigned char dsineye[4];  /* Eyecatcher (set by caller)           */
  short int dsinlen;         /* Length of area filled in             */
  unsigned char dsinver;     /* Version of data                      */
  unsigned char dsinflg1;    /* Flag bytes                           */
  long long int dsinrecn;    /* Record number of returned record     */
  long long int dsinlglr;    /* Record number of returned record     */
  long long int dsinstke;    /* Time stamp of record PUT if          */
  int dsindsnu;              /* JES data set number where record     */
  int dsinjbno;              /* JES binary job number of owning job  */
  unsigned char dsinjbid[8]; /* JES job id of owning job             */
  int dsinnins;              /* Next instream dataset nr    @Z13LIST */
  int _filler1;              /* Reserved                    @Z13LIST */
};

/* Values for field "dsinver" */
#define dsinver1 1    /* Version 1 of IAZDSINF              */
#define dsinverc 0x01 /* Current version of data            */

/* Values for field "dsinflg1" */
#define dsin1trc 0x80 /* Returned area truncated            */
#define dsin1rsk 0x40 /* Records skipped due to      @Z23BK */
#define dsin1err 0x20 /* Current record is error   @Z21LCJL */
#define dsin1war 0x10 /* Current record is warning @Z21LCJL */
#define dsin1jec 0x08 /* Start or continuation     @Z21LCJL */

/* Values for field "_filler1" */
#define dsinsiz1 0x38 /* Version 1 size of area             */
#define dsinsize 0x38 /* Size of area                       */

#endif

#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
