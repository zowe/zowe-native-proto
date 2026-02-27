#ifdef __open_xl__
#pragma pack(1)
#else
#pragma pack(packed)
#endif

#ifndef __exlst__
#define __exlst__

struct exlst {
  struct {
    unsigned char  _exlcodes;      /* VARIOUS EXLST CODES */
    unsigned int   _exlentrb : 24; /* ADDRESS OF EXIT     */
    } exlentra;
  };

#define exlcodes exlentra._exlcodes
#define exlentrb exlentra._exlentrb

/* Values for field "exlcodes" */
#define exlinact 0x00 /* INACTIVE ENTRY                                 */
#define exlihlab 0x01 /* INPUT HEADER LABEL                             */
#define exlohlab 0x02 /* OUTPUT HEADER LABEL                            */
#define exlitlab 0x03 /* INPUT TRAILER LABEL                            */
#define exlotlab 0x04 /* OUTPUT TRAILER LABEL                           */
#define exldcbex 0x05 /* DCB EXIT                                       */
#define exleovex 0x06 /* END OF VOLUME EXIT                             */
#define exlrjfcb 0x07 /* JFCB ADDR FOR RDJFCB, OPJ  @L2A                */
#define exlustot 0x0A /* USER''S TOTALING AREA ADDRESS                  */
#define exlblcnt 0x0B /* UNEQUAL BLOCK COUNT EXIT                       */
#define exldfrit 0x0C /* DEFER PROCESSING OF USER INPUT                 */
#define exldfnit 0x0D /* DEFER PROCESSING A NONSTANDARD                 */
#define exlfcbim 0x10 /* DEFINE AN FCB IMAGE                            */
#define exldcbab 0x11 /* ABEND EXIT                                     */
#define exlpdab  0x12 /* ADDR OF QSAM PARALLEL DATA ACCESS BLOCK   @L3A */
#define exlarl   0x13 /* RETRIEVE ALLOCATION INFORMATION           @L3A */
#define exluda   0x14 /* USAGE DESCRIPTION AREA                    @P1A */
#define exljfcbe 0x15 /* ADDR OF JFCBE EXIT ROUTINE (LIKE X'05')   @L3A */
#define exlload  0x16 /* PDSE LOAD MODULE ACCESS INFORMATION       @L4C */
#define exldcbsl 0x17 /* VOLUME SELECTION EXIT                     @L1A */
#define exldcbsc 0x18 /* VOLUME SECURITY EXIT                      @L1A */
#define exliopid 0x19 /* POINTER TO IOPID                          @L2A */
#define exllaste 0x80 /* LAST ENTRY IN LIST                             */

/* Values for field "exlentrb" */
#define exllenth 0x04 /* LENGTH OF DSECT (ONE ENTRY)                    */

#endif

#ifdef __open_xl__
#pragma pack()
#else
#pragma pack(reset)
#endif
