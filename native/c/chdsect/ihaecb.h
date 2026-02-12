#if defined(__MVS__)
#pragma pack(packed)
#endif

#ifndef __ecb__
#define __ecb__

struct ecb
{
  struct
  {
    struct
    {
      struct
      {
        unsigned char _ecbcc; /* -            COMPLETION CODE BYTE */
        struct
        {
          unsigned char _filler1[2]; /* -          FIRST TWO BYTES OF ECBEVTBA      @XM06072   */
          unsigned char _ecbbyte3;   /* -            THIRD BYTE OF ECBEVTBA (MDC303)  @XM06072 */
        } ecbcccnt;
      } ecbextb;
    } ecbevtb;
  } ecbrb;
};

#define ecbcc ecbrb.ecbevtb.ecbextb._ecbcc
#define ecbbyte3 ecbrb.ecbevtb.ecbextb.ecbcccnt._ecbbyte3

/* Values for field "ecbcc" */
#define ecbwait 0x80  /* -        WAITING FOR COMPLETION OF THE EVENT       */
#define ecbpost 0x40  /* -        THE EVENT HAS COMPLETED                   */
#define ecbunwt 0x30  /* -        ECB is "unwaited". (Normally used by      */
#define ecbnorm 0x7F  /* -        CHANNEL PROGRAM HAS TERMINATED WITHOUT    */
#define ecbperr 0x41  /* -        CHANNEL PROGRAM HAS TERMINATED WITH       */
#define ecbdaea 0x42  /* -        CHANNEL PROGRAM HAS TERMINATED BECAUSE A  */
#define ecbabend 0x43 /* -        I/O ABEND CONDITION OCCURRED FOR ERROR    */
#define ecbincpt 0x44 /* -        CHANNEL PROGRAM HAS BEEN INTERCEPTED @P1C */
#define ecbreprg 0x48 /* -        REQUEST ELEMENT FOR CHANNEL PROGRAM HAS   */
#define ecbehalt 0x48 /* -        ENABLE COMMAND HALTED, OR I/O OPERATION   */
#define ecberpab 0x4B /* -        ONE OF THE FOLLOWING ERRORS OCCURRED      */
#define ecberper 0x4F /* -        ERROR RECOVERY ROUTINES HAVE BEEN ENTERED */
#define ecbseteo 0x70 /* -        THE SETEOF MACRO WAS ISSUED IN THE        */
#define ecbdmqds 0x5C /* -        CONGESTED DESTINATION MESSAGE QUEUE DATA  */
#define ecbseqer 0x58 /* -        SEQUENCE ERROR  (TCAM)                    */
#define ecbinvmd 0x54 /* -        INVALID MESSAGE DESTINATION  (TCAM)       */
#define ecbwkovr 0x52 /* -        WORK AREA OVERFLOW  (TCAM)                */
#define ecbnomsg 0x50 /* -        MESSAGE WAS NOT FOUND WHEN READ MACRO     */
#define ecbdtraq 0x40 /* -        DATA IS ON READ-AHEAD QUEUE  (TCAM)       */
#define ecbeoq 0x02   /* -        END-OF-QUEUE CONDITION (NOT END-OF-FILE)  */
#define ecbraqmt 0x01 /* -        READ-AHEAD QUEUE EMPTY, BUT DESTINATION   */

/* Values for field "ecbbyte3" */
#define ecbextnd 0x03 /* -        ECB EXTENSION EXISTS (OS/VS2)             */
#define ecbevnt 0x01  /* -        EXTENDED FORMAT ECB  (MDC304)    @XM06072 */

#endif

#ifndef __ecbe__
#define __ecbe__

struct ecbe
{
  struct
  {
    unsigned char _ecbeval;  /* -            FUNCTION CODE  (MDC310)          @Z40EP9A */
    unsigned char _ecbeflg1; /* -            FLAG BYTE                        @G860PXB */
    unsigned char _ecberes2; /* -            RESERVED  (MDC313)               @Z40EP9A */
    unsigned char _ecberes3; /* -            RESERVED  (MDC314)               @Z40EP9A */
  } ecbedesc;
  struct
  {
    void *__ptr32 _ecbeexad; /* -            POST EXIT ADDRESS                @G860PXB */
  } ecbepind;
};

#define ecbeval ecbedesc._ecbeval
#define ecbeflg1 ecbedesc._ecbeflg1
#define ecberes2 ecbedesc._ecberes2
#define ecberes3 ecbedesc._ecberes3
#define ecbeexad ecbepind._ecbeexad

/* Values for field "ecbeval" */
#define ecbeexit 0x01 /* -        EXIT ROUTINE REQUEST  (MDC311)   @Z40EP9A */

/* Values for field "ecbeflg1" */
#define ecbemode 0x80 /* ADDRESSING MODE OF EXIT --                         */

#endif

#if defined(__MVS__)
#pragma pack(reset)
#endif