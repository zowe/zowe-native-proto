#ifndef __jqry___header__
#define __jqry___header__

// TODO(Kelosky): this needs changed to match ibm-clang conversion
#if (defined(__IBMCPP__) || defined(__IBMC__))
#if defined(__clang__)
#pragma pack(1)
#else
#pragma pack(packed)
#endif
#endif

struct jqry___header
{
  unsigned char jqryid[4]; /* Control block identifier (JQRYCID) */
  short int jqryvers;      /* Version number.  Current version   */
  short int _filler1;      /* Reserved                           */
  int jqrylen;             /* Length of data returned by the     */
  int jqry___num___subsys; /* Number of subsystems for which     */
};

#endif

#ifndef __jqry___subsys___entry__
#define __jqry___subsys___entry__

struct jqry___subsys___entry
{
  unsigned char jqry___subsys___name[4]; /* Name of the subsystem               */
  unsigned char jqry___ssid;             /* Subsystem ID                   @P3A */
  unsigned char _filler1[7];             /* Reserved                            */
  struct
  {
    unsigned char _jqry___status1; /* Subsystem flags - byte 1       @P2A */
    unsigned char _jqry___status2; /* Subsystem flags - byte 2       @P2A */
  } jqry___status;
  short int jqry___num___vt; /* Number of vector tables associated  */
};

#define jqry___status1 jqry___status._jqry___status1
#define jqry___status2 jqry___status._jqry___status2

/* Values for field "jqry___ssid" */
#define jqry___ssid___unknown 0x00 /* SSID value when unknown        @P3A */
#define jqry___ssid___jes2 0x02    /* SSID value when JES2           @P3A */
#define jqry___ssid___jes3 0x03    /* SSID value when JES3           @P3A */

/* Values for field "jqry___status1" */
#define jqry___primary1 0x80           /* Subsystem is the primary            */
#define jqry___dynamic1 0x40           /* Subsystem is dynamic           @P2A */
#define jqry___dynssi___commands1 0x20 /* Subsystem responds to the SETSSI    */
#define jqry___active1 0x10            /* Subsystem is active            @P2A */
#define jqry___eventrtn___loaded 0x08  /* Subsystem has a subsystem event     */

/* Values for field "jqry___status2" */
#define jqry___incomplete2 0x01 /* Data for this subsystem may be      */

/* Values for field "jqry___status" */
#define jqry___primary 0x8000           /* Subsystem is the primary subsystem  */
#define jqry___dynamic 0x4000           /* Subsystem is dynamic                */
#define jqry___dynssi___commands 0x2000 /* Subsystem responds to the SETSSI    */
#define jqry___active 0x1000            /* Subsystem is active                 */
#define jqry___incomplete 0x01          /* Data for this subsystem may be      */

#endif

#ifndef __jqry___vt___entry__
#define __jqry___vt___entry__

struct jqry___vt___entry
{
  int jqry___vt___loc;             /* Vector table locator.  This is a    */
  unsigned char jqry___vt___flags; /* Vector table flags             @01C */
  unsigned char _filler1[3];       /* Reserved                            */
  unsigned char _filler2[4];       /* Reserved                       @D1C */
  struct
  {
    unsigned char _jqry___vt___func___codes[32]; /* Bit mask indicating support    @01C */
  } jqry___vt___func___list;
  unsigned char _filler3[8]; /* Reserved                            */
};

#define jqry___vt___func___codes jqry___vt___func___list._jqry___vt___func___codes

/* Values for field "jqry___vt___flags" */
#define jqry___vt___active 0x80        /* This vector table is being used */
#define jqry___vt___ssi___managed 0x40 /* Vector table is SSI-managed     */

#pragma pack(reset)

#endif
