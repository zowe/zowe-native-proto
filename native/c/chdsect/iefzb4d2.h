#pragma pack(packed)

#ifndef __svc99kys__
#define __svc99kys__

struct svc99kys {
  int            noexist; /* THIS IS NOT A REAL FIELD */
  };

/* Values for field "noexist" */
#define dalddnam           0x01   /* DDNAME                                  */
#define daldsnam           0x02   /* DSNAME                                  */
#define dalmembr           0x03   /* MEMBER NAME                             */
#define dalstats           0x04   /* DATA SET STATUS                         */
#define dalndisp           0x05   /* DATA SET NORMAL DISPOSITION             */
#define dalcdisp           0x06   /* DATA SET CONDITIONAL DISP               */
#define daltrk             0x07   /* TRACK SPACE TYPE                        */
#define dalcyl             0x08   /* CYLINDER SPACE TYPE                     */
#define dalblkln           0x09   /* AVERAGE DATA BLOCK LENGTH               */
#define dalprime           0x0A   /* PRIMARY SPACE QUANTITY                  */
#define dalsecnd           0x0B   /* SECONDARY SPACE QUANTITY                */
#define daldir             0x0C   /* DIRECTORY SPACE QUANTITY                */
#define dalrlse            0x0D   /* UNUSED SPACE RELEASE                    */
#define dalspfrm           0x0E   /* CONTIG,MXIG,ALX SPACE FORMAT            */
#define dalround           0x0F   /* WHOLE CYLINDER (ROUND) SPACE            */
#define dalvlser           0x10   /* VOLUME SERIAL                           */
#define dalprivt           0x11   /* PRIVATE VOLUME                          */
#define dalvlseq           0x12   /* VOL SEQUENCE NUMBER                     */
#define dalvlcnt           0x13   /* VOLUME COUNT                            */
#define dalvlrds           0x14   /* VOLUME REFERENCE TO DSNAME              */
#define dalunit            0x15   /* UNIT DESCRIPTION                        */
#define daluncnt           0x16   /* UNIT COUNT                              */
#define dalparal           0x17   /* PARALLEL MOUNT                          */
#define dalsysou           0x18   /* SYSOUT                                  */
#define dalspgnm           0x19   /* SYSOUT PROGRAM NAME                     */
#define dalsfmno           0x1A   /* SYSOUT FORM NUMBER                      */
#define daloutlm           0x1B   /* OUTPUT LIMIT                            */
#define dalclose           0x1C   /* UNALLOCATE AT CLOSE                     */
#define dalcopys           0x1D   /* SYSOUT COPIES                           */
#define dallabel           0x1E   /* LABEL TYPE                              */
#define daldsseq           0x1F   /* DATA SET SEQUENCE NUMBER                */
#define dalpaspr           0x20   /* PASSWORD PROTECTION                     */
#define dalinout           0x21   /* INPUT ONLY OR OUTPUT ONLY               */
#define dalexpdt           0x22   /* 2 DIGIT YEAR EXPIRATION DATE            */
#define dalretpd           0x23   /* RETENTION PERIOD                        */
#define daldummy           0x24   /* DUMMY ALLOCATION                        */
#define dalfcbim           0x25   /* FCB IMAGE-ID                            */
#define dalfcbav           0x26   /* FCB FORM ALIGNMENT,IMAGE VERIFY         */
#define dalqname           0x27   /* QNAME ALLOCATION                        */
#define dalterm            0x28   /* TERMINAL ALLOCATION                     */
#define dalucs             0x29   /* UNIVERSAL CHARACTER SET                 */
#define dalufold           0x2A   /* UCS FOLD MODE                           */
#define daluvrfy           0x2B   /* UCS VERIFY CHARACTER SET                */
#define daldcbds           0x2C   /* DCB DSNAME REFERENCE                    */
#define daldcbdd           0x2D   /* DCB DDNAME REFERENCE                    */
#define dalbfaln           0x2E   /* BUFFER ALIGNMENT                        */
#define dalbftek           0x2F   /* BUFFERING TECHNIQUE                     */
#define dalblksz           0x30   /* BLOCKSIZE                               */
#define dalbufin           0x31   /* NUMBER OF INPUT BUFFERS                 */
#define dalbufl            0x32   /* BUFFER LENGTH                           */
#define dalbufmx           0x33   /* MAXIMUM NUMBER OF BUFFERS               */
#define dalbufno           0x34   /* NUMBER OF DCB BUFFERS                   */
#define dalbufof           0x35   /* BUFFER OFFSET                           */
#define dalbufou           0x36   /* NUMBER OF OUTPUT BUFFERS                */
#define dalbufrq           0x37   /* NUMBER OF GET MACRO BUFFERS             */
#define dalbufsz           0x38   /* LINE BUFFER SIZE                        */
#define dalcode            0x39   /* PAPER TAPE CODE                         */
#define dalcpri            0x3A   /* SEND/RECEIVE PRIORITY                   */
#define dalden             0x3B   /* TAPE DENSITY                            */
#define daldsorg           0x3C   /* DATA SET ORGANIZATION                   */
#define daleropt           0x3D   /* ERROR OPTIONS                           */
#define dalgncp            0x3E   /* NO. OF GAM I/O BEFORE WAIT              */
#define dalintvl           0x3F   /* POLLING INTERVAL                        */
#define dalkylen           0x40   /* DATA SET KEYS LENGTH                    */
#define dallimct           0x41   /* SEARCH LIMIT                            */
#define dallrecl           0x42   /* LOGICAL RECORD  LENGTH                  */
#define dalmode            0x43   /* CARD READER/PUNCH MODE                  */
#define dalncp             0x44   /* NO. READ/WRITE BEFORE CHECK             */
#define daloptcd           0x45   /* OPTIONAL SERVICES                       */
#define dalpcir            0x46   /* RECEIVING PCI                           */
#define dalpcis            0x47   /* SENDING PCI                             */
#define dalprtsp           0x48   /* PRINTER LINE SPACING                    */
#define dalrecfm           0x49   /* RECORD FORMAT                           */
#define dalrsrvf           0x4A   /* FIRST BUFFER RESERVE                    */
#define dalrsrvs           0x4B   /* SECONDARY BUFFER RESERVE                */
#define dalsowa            0x4C   /* TCAM USER WORK AREA SIZE                */
#define dalstack           0x4D   /* STACKER BIN                             */
#define dalthrsh           0x4E   /* MESSAGE QUEUE PERCENTAGE                */
#define daltrtch           0x4F   /* TAPE RECORDING TECHNOLOGY @T1C          */
#define dalpassw           0x50   /* PASSWORD                                */
#define dalipltx           0x51   /* IPL TEXT ID                             */
#define dalperma           0x52   /* PERMANENTLY ALLOCATED ATTRIB            */
#define dalcnvrt           0x53   /* CONVERTIBLE ATTRIBUTE                   */
#define daldiagn           0x54   /* OPEN/CLOSE/EOV DIAGNOSTIC TRACE         */
#define dalrtddn           0x55   /* RETURN DDNAME                           */
#define dalrtdsn           0x56   /* RETURN DSNAME                           */
#define dalrtorg           0x57   /* RETURN D.S. ORGANIZATION                */
#define dalsuser           0x58   /* SYSOUT REMOTE USER                      */
#define dalshold           0x59   /* SYSOUT HOLD QUEUE                       */
#define dalfunc            0x5A   /* D.S. TYPE FOR 3525 CARD DEVICE          */
#define dalfrid            0x5B   /* IMAGELIB MEMBER FOR SHARK               */
#define dalssreq           0x5C   /* SUBSYSTEM REQUEST                       */
#define dalrtvol           0x5D   /* RETURN VOLUME SERIAL                    */
#define dalmsvgp           0x5E   /* MSVGP FOR 3330V        @Y30LPPD         */
#define dalssnm            0x5F   /* SUBSYSTEM NAME REQUEST @G29AN2F         */
#define dalssprm           0x60   /* SUBSYSTEM PARAMETERS   @G29AN2F         */
#define dalprot            0x61   /* RACF PROTECT FEATURE   @G32HPPJ         */
#define dalssatt           0x62   /* SUBSYSTEM ATTRIBUTE    @ZA25778         */
#define dalusrid           0x63   /* SYSOUT USER ID         @G860P44         */
#define dalburst           0x64   /* BURSTER-TRIMMER-STACKER    @H1A         */
#define dalchars           0x65   /* CHAR ARRANGEMENT TABLE     @H1A         */
#define dalcopyg           0x66   /* COPY GROUP VALUES          @H1A         */
#define dalfform           0x67   /* FLASH FORMS OVERLAY        @H1A         */
#define dalfcnt            0x68   /* FLASH FORMS OVERLAY COUNT  @H1A         */
#define dalmmod            0x69   /* COPY MODIFICATION MODULE   @H1A         */
#define dalmtrc            0x6A   /* TABLE REFERENCE CHARACTER  @H1A         */
#define dallreck           0x6B   /* LRECL IN MULT OF 1K FORMAT @L1A         */
#define daldefer           0x6C   /* DEFER MOUNT UNTIL OPEN     @D1A         */
#define dalexpdl           0x6D   /* 4 DIGIT YEAR EXP. DATE     @L2A         */
#define dalbrtkn           0x6E   /* Browse token supplied      @D3A         */
#define dalinchg           0x6F   /* Volume Interchange                      */
#define dalovaff           0x70   /* Tell JES to override       @L6A         */
#define dalrtctk           0x71   /* Return Allocation Sysout   @L8A         */
#define dalkilo            0x72   /* BLKSIZE OF KILOBYTE        @L9A         */
#define dalmeg             0x73   /* BLKSIZE OF MEGABYTE        @L9A         */
#define dalgig             0x74   /* BLKSIZE OF GIGABYTE        @L9A         */
#define daluassr           0x75   /* Unauthorized subsystem     @LAA         */
#define dalsmshr           0x76   /* unitname to be honored on an            */
#define dalunqds           0x77   /* Uniquely allocated temporary            */
#define dalreqiefopz       0x78   /* Request IEFOPZ processing  @02C         */
#define dalinsdd           0x79   /* Insulated DD request       @03A         */
#define dalnosec           0x7A   /* Bypass security checking   @03A         */
#define dalretinfo         0x7B   /* Return allocation information           */
#define dalretiefopznewdsn 0x7C   /* Return IEFOPZ-new data set name         */
#define dalretiefopznewvol 0x7D   /* Return IEFOPZ-new volume serial         */
#define dalacode           0x8001 /* ACCESSIBILITY CODE         @L1A         */
#define daloutpt           0x8002 /* OUTPUT REFERENCE           @H1A         */
#define dalcntl            0x8003 /* CNTL                               @D1A */
#define dalstcl            0x8004 /* STORCLAS                                */
#define dalmgcl            0x8005 /* MGMTCLAS                                */
#define daldacl            0x8006 /* DATACLAS                                */
#define dalreco            0x800B /* RECORG                                  */
#define dalkeyo            0x800C /* KEYOFF                                  */
#define dalrefd            0x800D /* REFDD                                   */
#define dalsecm            0x800E /* SECMODEL                                */
#define dallike            0x800F /* LIKE                                    */
#define dalavgr            0x8010 /* AVGREC                                  */
#define daldsnt            0x8012 /* DSNTYPE                          @L1A   */
#define dalspin            0x8013 /* SPIN                             @L2A   */
#define dalsegm            0x8014 /* SEGMENT                          @L2A   */
#define dalpath            0x8017 /* PATH                               @L4A */
#define dalpopt            0x8018 /* PATHOPTS                           @L4A */
#define dalpmde            0x8019 /* PATHMODE                           @L4A */
#define dalpnds            0x801A /* PATHDISP - Normal Disposition      @L4A */
#define dalpcds            0x801B /* PATHDISP - Conditional Disposition @L4A */
#define dalrls             0x801C /* RLS - Record Level Sharing         @L5A */
#define dalfdat            0x801D /* FILEDATA - file organization       @L6A */
#define dallgst            0x801F /* LGSTREAM                           @02A */
#define daldccs            0x8020 /* CCSID                              @03A */
#define dalbslm            0x8022 /* BLKSZLIM                           @04A */
#define dalkyl1            0x8023 /* KEYLABL1                           @P7C */
#define dalkyl2            0x8024 /* KEYLABL2                           @P7C */
#define dalkyc1            0x8025 /* KEYENCD1                           @P7C */
#define dalkyc2            0x8026 /* KEYENCD2                           @P7C */
#define daleatt            0x8028 /* EATTR                              @L9A */
#define dalfrvl            0x8029 /* FREEVOL                            @LAA */
#define dalspi2            0x802A /* SPIN second parm, SPIN INTERVAL    @PAC */
#define dalsyml            0x802B /* SYMLIST ON DD                      @06M */
#define daldsnv            0x802C /* DSNTYPE version                    @06M */
#define dalmaxg            0x802D /* MAXGENS                            @06C */
#define dalgdgo            0x802E /* GDGORDER - GDG-all concatenation order  */
#define dalroac            0x8030 /* ROACCESS - read-only access        @LFA */
#define dalroa2            0x8031 /* ROACCESS - second parm             @LFA */
#define daldkyl            0x8032 /* DSKEYLBL - Data set encryption     @07A */
#define dccddnam           0x01   /* DDNAMES                                 */
#define dccpermc           0x04   /* PERMANENTLY CONCATENATED                */
#define dccinsdd           0x79   /* Concatenate Insulated DDs  @03A         */
#define ddcddnam           0x01   /* DDNAME                                  */
#define ddcinsdd           0x79   /* Deconcatenate Insulated DD @03A         */
#define dinddnam           0x01   /* DDNAME                                  */
#define dindsnam           0x02   /* DSNAME                                  */
#define dinrtddn           0x04   /* RETURN DDNAME                           */
#define dinrtdsn           0x05   /* RETURN DSNAME                           */
#define dinrtmem           0x06   /* RETURN MEMBER NAME                      */
#define dinrtsta           0x07   /* RETURN DATA SET STATUS                  */
#define dinrtndp           0x08   /* RETURN NORMAL DISPOSITION               */
#define dinrtcdp           0x09   /* RETURN CONDITIONAL DISP                 */
#define dinrtorg           0x0A   /* RETURN D.S. ORGANIZATION                */
#define dinrtlim           0x0B   /* RETURN # TO NOT-IN-USE LIMIT            */
#define dinrtatt           0x0C   /* RETURN DYN. ALLOC ATTRIBUTES            */
#define dinrtlst           0x0D   /* RETURN LAST ENTRY INDICATION            */
#define dinrttyp           0x0E   /* RETURN S.D. TYPE INDICATION             */
#define dinrelno           0x0F   /* RELATIVE REQUEST NUMBER                 */
#define dinrtvol           0x10   /* Return First Volser        @L7A         */
#define dinrtddx           0x11   /* Return DDname extended     @LCA         */
#define dinrlpos           0x12   /* Return Relative Position   @LCA         */
#define dinrpnam           0x13   /* Return SYSOUT program name @04A         */
#define dinrcntl           0xC003 /* CNTL                               @D1A */
#define dinrstcl           0xC004 /* STORCLAS                                */
#define dinrmgcl           0xC005 /* MGMTCLAS                                */
#define dinrdacl           0xC006 /* DATACLAS                                */
#define dinrreco           0xC00B /* RECORG                                  */
#define dinrkeyo           0xC00C /* KEYOFF                                  */
#define dinrrefd           0xC00D /* REFDD                                   */
#define dinrsecm           0xC00E /* SECMODEL                                */
#define dinrlike           0xC00F /* LIKE                                    */
#define dinravgr           0xC010 /* AVGREC                                  */
#define dinrdsnt           0xC012 /* DSNTYPE                           @L1A  */
#define dinrspin           0xC013 /* SPIN                              @L2A  */
#define dinrsegm           0xC014 /* SEGMENT                           @L2A  */
#define dinrpath           0xC017 /* PATH                               @L4A */
#define dinrpopt           0xC018 /* PATHOPTS                           @L4A */
#define dinrpmde           0xC019 /* PATHMODE                           @L4A */
#define dinrpnds           0xC01A /* NORMAL PATHDISP                    @L4A */
#define dinrcnds           0xC01B /* CONDITIONAL PATHDISP               @L4A */
#define dinrpcds           0xC01B /* CONDITIONAL PATHDISP               @P5A */
#define dinrfdat           0xC01D /* FILEDATA                           @L6A */
#define dinreatt           0xC028 /* EATTR                              @LGA */
#define dinrspi2           0xC02A /* SPIN interval                      @LBA */
#define dinrsyml           0xC02B /* SYMLIST                            @PBA */
#define dinrdsnv           0xC02C /* DSNTYPE version                    @LDA */
#define dinrmaxg           0xC02D /* MAXGENS                            @06C */
#define dinrgdgo           0xC02E /* GDGORDER                           @LEA */
#define dinrroac           0xC030 /* ROACCESS - first parm              @LFA */
#define dinrroa2           0xC031 /* ROACCESS - second parm             @LFA */
#define dinrdkyl           0xC032 /* DSKEYLBL                           @07A */
#define dinpath            0x8017 /* PATH                               @L4A */
#define dritcbad           0x01   /* TCB ADDRESS                             */
#define dricurnt           0x02   /* CURRENT TASK OPTION                     */
#define ddnddnam           0x01   /* DDNAME                                  */
#define ddnrtdum           0x02   /* RETURN DUMMY D.S. INDICATION            */
#define dunddnam           0x01   /* DDNAME                                  */
#define dundsnam           0x02   /* DSNAME                                  */
#define dunmembr           0x03   /* MEMBER NAME                             */
#define dunovdsp           0x05   /* OVERRIDING DISPOSITION                  */
#define dununalc           0x07   /* UNALLOC OPTION                          */
#define dunremov           0x08   /* REMOVE OPTION                           */
#define dunovsnh           0x0A   /* OVERRIDING SYSOUT NOHOLD                */
#define dunovcls           0x18   /* OVERRIDING SYSOUT CLASS                 */
#define dunovsus           0x58   /* OVERRIDING SYSOUT NODE     @P2C         */
#define dunovshq           0x59   /* OVERRIDING SYSOUT HOLD QUEUE            */
#define dunovuid           0x63   /* Overriding SYSOUT User ID  @P2A         */
#define duninsdd           0x79   /* Unallocate Insulated DD    @03A         */
#define dunnosec           0x7A   /* Bypass security checking   @03A         */
#define dunspin            0x8013 /* SPIN                               @L3A */
#define dunpath            0x8017 /* PATH                               @L4A */
#define dunovpds           0x801A /* PATHDISP - Override Disposition   @L4A  */
#define dunspi2            0x802A /* SPIN                               @PAC */

#endif

#pragma pack(reset)
