#pragma pack(packed)
 
#ifndef __tlkup__
#define __tlkup__
 
struct tlkup {
  unsigned char  tlkeye[5];   /* Eyecatcher                    @Z22AM */
  unsigned char  _filler1;    /* Reserved                             */
  short int      tlklen;      /* Length of parameter list             */
  unsigned char  tlktblid[3]; /* Text table ID                        */
  unsigned char  tlklevel;    /* Level of text to extract             */
  unsigned char  tlkmflg[4];  /* Remaining flag reasons        @Z22AM */
  void          *tlkssobp;    /* SSOB address                         */
  void          *tlkdatap;    /* Address of SSI output data    @Z22AM */
  void          *tlkoutp;     /* Addr to output area where     @Z22AM */
  int            tlkoutl;     /* Output area length                   */
  int            tlkretcd;    /* Overall return code                  */
  };
 
/* Values for field "tlkretcd" */
#define tlkrsucc 0x00 /* Successful completion. For    @Z22AM */
#define tlkrmflg 0x04 /* Successful completion. For    @Z22AM */
#define tlkrnotf 0x08 /* Code/Flag setting not found          */
#define tlkrntbl 0x0C /* No text table pointer in SSI         */
#define tlkrbads 0x10 /* Bad SSOB ptr, SSI function,          */
#define tlkrbadt 0x14 /* Bad text table ID                    */
#define tlkrnout 0x18 /* Output data area pointer or          */
#define tlkrbade 0x1C /* Bad text table eyecatcher            */
#define tlkrbadl 0x20 /* Bad text table level requested       */
#define tlkrbady 0x24 /* Bad text table type for request      */
#define tlkupln  0x30 /* Length of TLKUP DSECT                */
#define tphzlen1 64   /* Job Phase text table          @Z22AM */
#define tphzlen2 64   /* Job Phase text table          @Z22AM */
#define tphzlen3 20   /* Job Phase text table          @Z22AM */
#define tdlylen1 20   /* Job Delay text table          @Z22AM */
#define tdlylen2 64   /* Job Delay text table          @Z22AM */
 
#endif
 
#pragma pack(reset)
