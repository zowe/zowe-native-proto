#pragma pack(1)

#ifndef __mcscsa__
#define __mcscsa__

struct mcscsa
{
  unsigned char mcscid[4]; /* Acronym 'MCSC'                      */
  unsigned char mcscver;   /* Version level                       */
  unsigned char mcscflgs;  /* Flags byte                          */
  short int mcscstor;      /* ALWAYS ZERO - CONTAINS NO VALID     */
  int mcsccnid;            /* Console ID of message owner         */
  void *__ptr32 mcscnuse;  /* ALWAYS ZERO - CONTAINS NO VALID     */
  int mcsctdep;            /* Total Message Queue Depth           */
  int mcscudep;            /* Message Queue Depth for Unsolicited */
  int mcscddep;            /* Message Queue Depth for Delivered   */
  int mcscpdep;            /* Maximum message queue depth         */
  unsigned char mcscmfrm;  /* Message format - (Note: the bit     */
  union
  {
    unsigned char _mcscqsta[4]; /* Queuing Status */
    struct
    {
      unsigned char _mcscmlim; /* Queuing Stopped by Memory Limit   */
      unsigned char _mcscdlim; /* Queuing Stopped by Queue Depth    */
      unsigned char _mcscintr; /* Queuing Stopped by Internal Error */
      unsigned char _mcscalrt; /* Queuing Reached Alert percentage  */
    } _mcscsa_struct1;
  } _mcscsa_union1;
  unsigned char mcscsusp;    /* Request to suspend the operator     */
  unsigned char _filler1[6]; /* Reserved                            */
  union
  {
    int _mcscflgs___cs; /* Flags field manipulated via Compare */
    struct
    {
      unsigned char _mcscflgs___cs1; /* Byte 1 */
      unsigned char _mcscflgs___cs2; /* Byte 2 */
      unsigned char _mcscflgs___cs3; /* Byte 3 */
      unsigned char _mcscflgs___cs4; /* Byte 4 */
    } _mcscsa_struct2;
  } _mcscsa_union2;
  void *__ptr32 mcscoext; /* Pointer to O.C.O extension          */
  unsigned char mcscend;  /* End of MCSCSA non-O.C.O portion     */
};

#define mcscqsta _mcscsa_union1._mcscqsta
#define mcscmlim _mcscsa_union1._mcscsa_struct1._mcscmlim
#define mcscdlim _mcscsa_union1._mcscsa_struct1._mcscdlim
#define mcscintr _mcscsa_union1._mcscsa_struct1._mcscintr
#define mcscalrt _mcscsa_union1._mcscsa_struct1._mcscalrt
#define mcscflgs___cs _mcscsa_union2._mcscflgs___cs
#define mcscflgs___cs1 _mcscsa_union2._mcscsa_struct2._mcscflgs___cs1
#define mcscflgs___cs2 _mcscsa_union2._mcscsa_struct2._mcscflgs___cs2
#define mcscflgs___cs3 _mcscsa_union2._mcscsa_struct2._mcscflgs___cs3
#define mcscflgs___cs4 _mcscsa_union2._mcscsa_struct2._mcscflgs___cs4

/* Values for field "mcscflgs" */
#define mcscpost 0x80 /* A post was done on the Alert ECB   */

/* Values for field "mcscmfrm" */
#define mcscdtim 0x80 /* Display timestamp                  */
#define mcscdjob 0x40 /* Display jobname                    */
#define mcscdsys 0x04 /* Display system name                */
#define mcscdx 0x02   /* Don't display system name and      */

/* Values for field "mcscflgs___cs1" */
#define mcscmessageecbisposted 0x80 /* A post was done on the Message ECB */

/* Values for field "mcscend" */
#define mcscacrn 0xD4C3E2C3 /* Acronym 'MCSC'                     */
#define mcscvers 1          /* Current version                    */
#define mcsc410 1           /* Version level for SP4.1.0          */
#define mcscsa___len 0x34

#endif

#pragma pack()
