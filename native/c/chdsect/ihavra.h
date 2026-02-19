#pragma pack(1)
 
#ifndef __vramap__
#define __vramap__
 
struct vramap {
  struct {
    char           _vrakey; /* KEY TO IDENTIFY THE DATA THAT FOLLOWS. THE */
    char           _vralen; /* LENGTH OF THE DATA THAT FOLLOWS.  THE      */
    } vrakl;
  char           vradat; /* VARIABLE LENGTH DATA. THIS DATA IS FOLLOWED */
  };
 
#define vrakey vrakl._vrakey
#define vralen vrakl._vralen
 
/* Values for field "vradat" */
#define vralenkl 0x02 /* LENGTH OF THE VRAKL FIELD (VRAKEY AND            */
#define vracom   1    /* THE VRADAT DATA IS THE 5-BYTE EBCDIC COMPONENT   */
#define vrasc    2    /* THE DATA IS EBCDIC TEXT TO IDENTIFY THE          */
#define vralvl   3    /* THE DATA IS THE EBCDIC LEVEL FOR THE FAILING     */
#define vradt    4    /* THE DATA IS THE EBCDIC ASSEMBLY DATE FOR THE     */
#define vraptf   5    /* THE DATA IS THE 7-BYTE EBCDIC PTF, SU, OR        */
#define vrarc    6    /* THE DATA IS A HEXADECIMAL RETURN OR REASON       */
#define vraqvod  7    /* THE DATA IS THE REGISTER 15 AND ERROR PORTIONS   */
#define vraqerr  8    /* THIS KEY INDICATES A QUEUE ERROR FOR THE         */
#define vralvls  9    /* THE DATA IS THE EBCDIC SYSTEM RELEASE OR         */
#define vrarrp   16   /* ('10'X) THE DATA IS THE HEXADECIMAL RECOVERY     */
#define vracbm   17   /* ('11'X) THE DATA IS THE MAPPING MACRO NAME       */
#define vracb    18   /* ('12'X) THE DATA IS THE HEXADECIMAL CONTENTS     */
#define vracbf   19   /* ('13'X) THE DATA IS THE NAME OF A CONTROL        */
#define vracba   20   /* ('14'X) THE DATA IS THE 4 BYTE ADDRESS OF        */
#define vracbo   21   /* ('15'X) THE DATA IS THE OFFSET OF A CONTROL      */
#define vracbl   22   /* ('16'X) THE DATA IS THE LENGTH OF THE CONTROL    */
#define vracbi   24   /* ('18'X) THE DATA IS A ONE BYTE CONTROL BLOCK     */
#define vracbia  25   /* ('19'X) THE DATA IS A ONE BYTE C.B. ID NUMBER    */
#define vracbi2  26   /* ('1A'X) THE DATA IS A ONE BYTE CONTROL BLOCK     */
#define vrapli   32   /* ('20'X) THE DATA IS EBCDIC TEXT TO IDENTIFY      */
#define vrapl    33   /* ('21'X) THE DATA IS THE HEXADECIMAL CONTENTS     */
#define vrafpi   34   /* ('22'X) THE DATA IS EBCDIC TEXT TO IDENTIFY      */
#define vrafp    35   /* ('23'X) THE DATA IS THE HEXADECIMAL CONTENTS     */
#define vrapa    36   /* ('24'X) THE DATA DESCRIBES THE EXECUTION PATH    */
#define vrap2    37   /* ('25'X) THE DATA DESCRIBES THE EXECUTION         */
#define vralk    38   /* ('26'X) THE DATA IS THE EBCDIC NAME OF A         */
#define vrawai   39   /* ('27'X) THE DATA IS EBCDIC TEXT TO IDENTIFY      */
#define vrawa    40   /* ('28'X) THE DATA IS THE HEXADECIMAL CONTENTS     */
#define vrawap   41   /* ('29'X) THE DATA IS THE ADDRESS OF A WORK AREA   */
#define vralbl   48   /* ('30'X) THE DATA IS AN EBCDIC LABEL              */
#define vrarrl   49   /* ('31'X) THE DATA IS THE LABEL OF THE             */
#define vramid   51   /* ('33'X) THE DATA IS AN EBCDIC MESSAGE ID FOR     */
#define vramsg   52   /* ('34'X) THE DATA IS EBCDIC MESSAGE TEXT FOR      */
#define vraerr   53   /* ('35'X) THE DATA IS EBCDIC INFORMATION ABOUT     */
#define vraehx   54   /* ('36'X) THE DATA IS HEXADECIMAL INFORMATION      */
#define vrahid   55   /* ('37'X) THE DATA IS AN EBCDIC HEADER TO          */
#define vrahex   56   /* ('38'X) THE DATA IS HEXADECIMAL INFORMATION      */
#define vraebc   57   /* ('39'X) THE DATA IS EBCDIC INFORMATION           */
#define vraaid   58   /* ('3A'X) THE DATA IS THE 2-BYTE HEXADECIMAL       */
#define vratcb   59   /* ('3B'X) THE DATA IS THE ADDRESS OF THE TCB       */
#define vraca    60   /* ('3C'X) THE DATA IS THE ADDRESS OF THE CALLER    */
#define vracan   61   /* ('3D'X) THE DATA IS THE NAME OF THE MODULE       */
#define vraoa    64   /* ('40'X) THE DATA IS THE ORIGINAL HEXADECIMAL     */
#define vrapsw   65   /* ('41'X) THE DATA IS THE PSW FROM THE ORIGINAL    */
#define vrains   66   /* ('42'X) THE DATA IS THE FAILING INSTRUCTION      */
#define vraregs  67   /* ('43'X) THE DATA IS THE GENERAL PURPOSE          */
#define vrarega  68   /* ('44'X) THE DATA IS THE ADDRESS OF AN AREA       */
#define vraor15  69   /* ('45'X) THE DATA IS REGISTER 15 AT THE TIME      */
#define vradsn   70   /* ('46'X) THE DATA IS THE EBCDIC NAME OF A         */
#define vradev   71   /* ('47'X) THE DATA IS THE EBCDIC NAME OF A DEVICE  */
#define vrasn    72   /* ('48'X) THE DATA IS HEXADECIMAL I/O SENSE DATA   */
#define vrast    73   /* ('49'X) THE DATA IS HEXADECIMAL I/O STATUS DATA  */
#define vrau     74   /* ('4A'X) THE DATA IS AN EBCDIC UNIT ADDRESS OR    */
#define vraccw   75   /* ('4B'X) THE DATA IS THE HEXADECIMAL CCW FOR      */
#define vracsw   76   /* ('4C'X) THE DATA IS THE HEXADECIMAL CSW FOR      */
#define vradvt   77   /* ('4D'X) THE DATA IS HEXADECIMAL DEVICE TYPE      */
#define vravol   78   /* ('4E'X) THE DATA IS AN EBCDIC VOLUME SERIAL      */
#define vrareq   80   /* ('50'X) THE DATA IS ONE OR MORE KEYS WHICH ARE   */
#define vraopt   81   /* ('51'X) THE DATA IS ONE OR MORE KEYS WHICH, IF   */
#define vraminsc 82   /* ('52'X) THE DATA IS A 2 BYTE MINIMUM COUNT OF    */
#define vradae   83   /* ('53'X) NO DATA IS ASSOCIATED WITH THIS KEY.     */
#define vraminsl 84   /* ('54'X) THE DATA IS A 2 BYTE MINIMUM LENGTH      */
#define vrafreg  96   /* ('60'X) THE DATA IS A 1 BYTE REGISTER NUMBER     */
#define vracscb  99   /* ('63'X) THE DATA IS THE CSCB CONTROL BLOCK WITH  */
#define vracscba 100  /* ('64'X) THE DATA IS THE ADDRESS OF THE CSCB      */
#define vrajob   101  /* ('65'X) THE DATA IS THE JOBNAME THAT FAILED.     */
#define vrastp   102  /* ('66'X) THE DATA IS THE STEPNAME THAT FAILED     */
#define vracmd   103  /* ('67'X) THE DATA IS AN EBCDIC TSO COMMAND OR     */
#define vrajcl   104  /* ('68'X) THE DATA IS A JCL STATEMENT              */
#define vranodae 105  /* ('69'X) NO DATA IS ASSOCIATED WITH THIS KEY.     */
#define vraepn   115  /* ('73'X) THE DATA IS THE NAME OF THE ENTRY        */
#define vraetf   119  /* ('77'X) THE DATA IS THE ADDRESS OF THE ENTRY     */
#define vractf   120  /* ('78'X) THE DATA IS THE ADDRESS OF THE CSECT     */
#define vraltf   121  /* ('79'X) THE DATA IS THE ADDRESS OF THE LOAD      */
#define vramo    122  /* ('7A'X) THE DATA IS THE HEXADECIMAL OFFSET       */
#define vrailo   123  /* ('7B'X) THE DATA IS THE HEXADECIMAL OFFSET OF    */
#define vraimo   124  /* ('7C'X) THE DATA IS THE HEXADECIMAL OFFSET OF    */
#define vrafid   125  /* ('7D'X) THE DATA IS THE EBCDIC FEATURE ID FOR    */
#define vrapid   126  /* ('7E'X) THE DATA IS THE EBCDIC PRODUCT ID FOR    */
#define vraiap   160  /* ('A0'X) THE DATA IS THE NAME OF AN ANALYTIC      */
#define vraial   161  /* ('A1'X) THE DATA IS A PARAMETER LIST FOR USE BY  */
#define vraicl   162  /* ('A2'X) THE DATA IS A PARAMETER LIST FOR USE IN  */
#define vraidp   163  /* ('A3'X) THE DATA IS THE NAME OF THE DUMP (OR     */
#define vralkwa  164  /* ('A4'X) THE DATA IS THE ADDRESS OF THE LOCKWORD  */
#define vrarrk   200  /* ('C8'X) THIS KEY AND KEYS 201 THRU 239 ('EF'X)   */
#define vrarrk1  201  /* ('C9'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk2  202  /* ('CA'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk3  203  /* ('CB'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk4  204  /* ('CC'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk5  205  /* ('CD'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk6  206  /* ('CE'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk7  207  /* ('CF'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk8  208  /* ('D0'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk9  209  /* ('D1'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk10 210  /* ('D2'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk11 211  /* ('D3'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk12 212  /* ('D4'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk13 213  /* ('D5'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk14 214  /* ('D6'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk15 215  /* ('D7'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk16 216  /* ('D8'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk17 217  /* ('D9'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk18 218  /* ('DA'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk19 219  /* ('DB'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk20 220  /* ('DC'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk21 221  /* ('DD'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk22 222  /* ('DE'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk23 223  /* ('DF'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk24 224  /* ('E0'X) KEY DEFINED BY THE RECOVERY ROUTINE      */
#define vrarrk25 225  /* ('E1'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk26 226  /* ('E2'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk27 227  /* ('E3'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk28 228  /* ('E4'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk29 229  /* ('E5'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk30 230  /* ('E6'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk31 231  /* ('E7'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk32 232  /* ('E8'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk33 233  /* ('E9'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk34 234  /* ('EA'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk35 235  /* ('EB'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk36 236  /* ('EC'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk37 237  /* ('ED'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk38 238  /* ('EE'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vrarrk39 239  /* ('EF'X) KEY DEFINED BY THE RECOVERY ROUTINE @D1A */
#define vraskp   250  /* ('FA'X) THIS KEY CAN BE USED TO SKIP TO A        */
#define vraend   255  /* ('FF'X) THE DATA FROM THIS KEY FIELD TO THE      */
#define efabs    1001 /* ('3E9'X) THE DATA IS THE SYSTEM ABEND CODE. @L1A */
#define efabu    1002 /* ('3EA'X) THE DATA IS THE USER ABEND CODE.   @L1A */
#define efldmd   1003 /* ('3EB'X) THE DATA IS THE FAILING LOAD MODULE     */
#define efcsct   1004 /* ('3EC'X) THE DATA IS THE FAILING CSECT NAME.@L1A */
#define efrexn   1005 /* ('3E9'X) THE DATA IS THE RECOVERY ROUTINE        */
#define efpsw    1011 /* ('3E9'X) THE DATA IS THE PSW REGISTER            */
#define e1c1d1c  1101 /* THIS KEY SHOULD NOT BE USED. IT IS RETAINED FOR  */
#define e1cid1c  1101 /* ('44D'X) THE DATA IS THE COMPONENT ID.      @01A */
#define e1sub1c  1102 /* ('44E'X) THE DATA IS THE SUBFUNCTION.       @L1A */
#define e1amd1c  1105 /* ('451'X) THE DATA IS THE ASSEMBLY DATE OF        */
#define e1vrs1c  1106 /* ('452'X) THE DATA IS THE VERSION OF THE          */
#define e1hrc1c  1108 /* ('454'X) THE DATA IS THE REASON OR RETURN        */
#define e1rrl1c  1110 /* ('456'X) THE DATA IS THE LABEL OF THE            */
#define e1cdb1c  1114 /* ('45A'X) THE DATA IS THE COMPONENT ID BASE       */
#define e1ccr1c  1116 /* ('45C'X) THE DATA ARE PROGRAM STATUS             */
#define e1hlh1c  1118 /* ('45E'X) COPY OF PSAHLHI-HIGHEST LOCK HELD       */
#define e1sup1c  1120 /* ('460'X) COPY OF PSASUPER (SUPERVISOR CONTROL    */
#define e1spn1c  1124 /* ('464'X) COPY OF LCCASPIN (PROCESSOR IS          */
#define e1fi1c   1126 /* ('466'X) THE DATA ARE THE 12 BYTES OF THE        */
#define e1frr1c  1128 /* ('468'X) THE DATA IS A COPY OF THE FRR PARAMETER */
#define e1asid1c 1130 /* ('46A'X) THE DATA IS THE ASID OF THE FAILING     */
#define e1orcc1c 1132 /* ('46C'X) THE DATA IS THE ORIGINAL COMPLETION     */
#define e1orrc1c 1134 /* ('46E'X) THE DATA IS THE ORIGINAL REASON         */
#define e1pids1c 1136 /* ('470'X) THE DATA IS THE PRODUCT/COMPONENT ID.   */
#define e2mcic   1203 /* ('4B3'X) THE DATA IS THE MACHINE CHECK           */
#define rinvld   0    /* ('0'X)   INVALID SYMPTOM.                   @L1A */
#define rabndsr  1    /* ('01'X) THE DATA IS THE SYSTEM ABEND CODE.  @L1A */
#define rabndur  2    /* ('02'X) THE DATA IS THE USER ABEND CODE.    @L1A */
#define rfldsr   3    /* ('03'X) THE DATA IS A FIELD NAME OR LABEL.  @L1A */
#define rlvlsr   4    /* ('04'X) THE DATA IS THE COMPONENT, SU, PP,       */
#define rmsgidr  5    /* ('05'X) MESSAGE IDENTIFIER.                 @L1A */
#define radrsr   6    /* ('06'X) ADDRESS OR OFFSET.                  @L1A */
#define rpcssr   7    /* ('07'X) THE DATA IS JCL, AN OPERATOR COMMAND     */
#define rpidsr   8    /* ('08'X) THE DATA IS A COMPONENT IDENTIFIER AS    */
#define rprcsr   9    /* ('09'X) THE DATA IS THE RETURN OR REASON         */
#define rptfsrr  10   /* ('0A'X) THE DATA IS A PTF IDENTIFIER.       @L1A */
#define rpzfsr   11   /* ('0B'X) THE DATA IS A SUPERZAP IDENTIFIER.  @L1A */
#define rregsr   12   /* ('0C'X) THE DATA ARE THE CONTENTS OF THE         */
#define rridsr   13   /* ('0D'X) MODULE, CSECT, ROUTINE NAME,             */
#define rstatr   14   /* ('0E'X) CSW, DSW STATUS.                    @L1A */
#define rvaluhr  15   /* ('0F'X) THE DATA IS HEXADECIMAL IN THE SOURCE    */
#define rvalucr  16   /* ('10'X) THE DATA IS CHARACTER IN THE SOURCE      */
#define rvalubr  17   /* ('11'X) THE DATA IS A FLAG FIELD IN THE SOURCE   */
 
#endif
 
#pragma pack()
