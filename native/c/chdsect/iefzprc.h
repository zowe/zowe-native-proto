#pragma pack(1)

#ifndef __iefzprc__
#define __iefzprc__

struct iefzprc
{
  short int _filler1; /* Dummy field */
};

/* Values for field "_filler1" */
#define prmlb___success 0                     /* X'000' IEFPRMLB completed successfully        */
#define prmlb___function___complete 0         /* X'000' Function completed                     */
#define prmlb___warning 4                     /* X'004' IEFPRMLB completed successfully with a */
#define prmlb___locks___held 8                /* X'008' Caller holds locks                     */
#define prmlb___request___failed 12           /* X'00C' IEFPRMLB request failed                */
#define prmlb___internal___error 16           /* X'010' IEFPRMLB internal error                */
#define prmlb___not___task___mode 20          /* X'014' Caller is not in TASK mode             */
#define prmlb___invalid___parameter___list 28 /* X'01C' Input parameter list is                */
#define prmlb___cross___memory 32             /* X'020' Caller is in Cross Memory Mode         */
#define prmlb___estae___setup___failed 36     /* X'024' ESTAE Setup failed                     */
#define prmlb___notauth___to___subpool 40     /* X'028' An unauthorized caller                 */
#define prmlb___rsn___ok 0                    /* X'000' Success reason code                    */
#define prmlb___dd___already___alloc 1        /* X'001' Specified DDname is already            */
#define prmlb___member___not___found 1        /* X'001' Specified member not found             */
#define prmlb___read___io___error 2           /* X'002' I/O error on member read               */
#define prmlb___open___error 3                /* X'003' Error opening parmlib dataset          */
#define prmlb___alloc___failed 4              /* X'004'Allocation of one of the logical        */
#define prmlb___concat___failed 5             /* X'005' Concatenation of the logical parmlib   */
#define prmlb___reader___load___failed 6      /* X'006' Load of the parmlib read routine       */
#define prmlb___unable___to___access___ds 7   /* X'007' Unable to access data set              */
#define prmlb___parmlib___still___open 8      /* X'008' The logical parmlib is still           */
#define prmlb___unalloc___failed 9            /* X'009' Unallocation of one of the logical     */
#define prmlb___read___buffer___full 10       /* X'00A' The input READ buffer is full and      */
#define prmlb___putline___error 11            /* X'00B' Putline processing abended. This       */
#define prmlb___bad___parameter 1             /* X'001' Bad parameter list passed to parmlib   */
#define prmlb___unknown___reason 2            /* X'002' Reason for failure is unknown          */
#define prmlb___plist___unaccessible 1        /* X'001' Unable to access the input             */
#define prmlb___listbuff___unaccessible 2     /* X'002' Unable to access the input             */
#define prmlb___msgbuff___unaccessible 3      /* X'003' Unable to access the input             */
#define prmlb___readbuff___unaccessible 4     /* X'004' Unable to access the input             */
#define prmlb___plist___s99txtpp___not0 5     /* X'005' S99TXTPP must be zero                  */
#define prmlb___msgbuff___format___error 6    /* X'006' Error in message buffer format         */
#define prmlb___readbuff___format___error 7   /* X'007' Error in read buffer format            */
#define prmlb___listbuff___format___error 8   /* X'008' Error in list buffer format            */
#define prmlb___s99rb___unaccessible 9        /* X'009' Unable to access the input S99RB       */
#define prmlb___rdsninfo___unaccessible 10    /* X'00A' Unable to access the                   */

#endif

#pragma pack()
