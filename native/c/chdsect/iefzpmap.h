#pragma pack(packed)
 
#ifndef __prm___list___buffer__
#define __prm___list___buffer__
 
struct prm___list___buffer {
  union {
    unsigned char  _prm___list___header[16]; /* Header */
    struct {
      char           _prm___list___version;     /* Version number. Must be set to PRM_List_Ver1 */
      unsigned char  _filler1[3];               /* Reserved                                     */
      int            _prm___num___parmlib___ds; /* Number of PARMLIB datasets in use by the     */
      int            _prm___list___buff___size; /* Input - Size of buffer including the header  */
      unsigned char  _filler2[4];               /* Reserved                                     */
      } _prm___list___buffer_struct1;
    } _prm___list___buffer_union1;
  union {
    struct {
      unsigned char  _prm___list___entries; /* Array of entries each mapped by */
      unsigned char  _filler3[55];
      } _prm___list___buffer_struct2;
    unsigned char  _prm___parmlib___ds___info[56]; /* PARMLIB data set record */
    struct {
      unsigned char  _prm___plib___dsn[44];   /* PARMLIB dataset name */
      unsigned char  _prm___plib___volser[6]; /* PARMLIB VOLSER       */
      unsigned char  _filler4[6];             /* Reserved             */
      } _prm___list___buffer_struct3;
    } _prm___list___buffer_union2;
  };
 
#define prm___list___header       _prm___list___buffer_union1._prm___list___header
#define prm___list___version      _prm___list___buffer_union1._prm___list___buffer_struct1._prm___list___version
#define prm___num___parmlib___ds  _prm___list___buffer_union1._prm___list___buffer_struct1._prm___num___parmlib___ds
#define prm___list___buff___size  _prm___list___buffer_union1._prm___list___buffer_struct1._prm___list___buff___size
#define prm___list___entries      _prm___list___buffer_union2._prm___list___buffer_struct2._prm___list___entries
#define prm___parmlib___ds___info _prm___list___buffer_union2._prm___parmlib___ds___info
#define prm___plib___dsn          _prm___list___buffer_union2._prm___list___buffer_struct3._prm___plib___dsn
#define prm___plib___volser       _prm___list___buffer_union2._prm___list___buffer_struct3._prm___plib___volser
 
/* Values for field "_filler4" */
#define prm___list___ver1              0x01 /* Version 1 indicator */
#define prm___list___current___version 0x01 /* Current Version     */
#define prm___list___buffer___len      0x48
 
#endif
 
#ifndef __prm___read___buffer__
#define __prm___read___buffer__
 
struct prm___read___buffer {
  union {
    unsigned char  _prm___read___header[24]; /* Read Buffer Header */
    struct {
      int            _prm___read___buff___size;     /* Input - Size of buffer including the header */
      int            _prm___records___read___count; /* Output - number of PARMLIB member           */
      int            _prm___buff___size___needed;   /* Output - size of buffer needed to contain   */
      int            _prm___total___records;        /* Output - Total number of records in the     */
      unsigned char  _filler1[8];                   /* Reserved                                    */
      } _prm___read___buffer_struct1;
    } _prm___read___buffer_union1;
  union {
    struct {
      unsigned char  _prm___records; /* Output: PARMLIB records area */
      unsigned char  _filler2[79];
      } _prm___read___buffer_struct2;
    unsigned char  _prm___record[80];           /* Output: array of PARMLIB records, each mapped */
    unsigned char  _prm___record___element[80]; /* One record                                    */
    struct {
      unsigned char  _prm___record___text[72]; /* First 72 characters of record (If */
      unsigned char  _prm___extraneous[8];     /* Sequence number                   */
      } _prm___read___buffer_struct3;
    } _prm___read___buffer_union2;
  };
 
#define prm___read___header          _prm___read___buffer_union1._prm___read___header
#define prm___read___buff___size     _prm___read___buffer_union1._prm___read___buffer_struct1._prm___read___buff___size
#define prm___records___read___count _prm___read___buffer_union1._prm___read___buffer_struct1._prm___records___read___count
#define prm___buff___size___needed   _prm___read___buffer_union1._prm___read___buffer_struct1._prm___buff___size___needed
#define prm___total___records        _prm___read___buffer_union1._prm___read___buffer_struct1._prm___total___records
#define prm___records                _prm___read___buffer_union2._prm___read___buffer_struct2._prm___records
#define prm___record                 _prm___read___buffer_union2._prm___record
#define prm___record___element       _prm___read___buffer_union2._prm___record___element
#define prm___record___text          _prm___read___buffer_union2._prm___read___buffer_struct3._prm___record___text
#define prm___extraneous             _prm___read___buffer_union2._prm___read___buffer_struct3._prm___extraneous
 
/* Values for field "prm___extraneous" */
#define prm___read___buffer___len 0x68
 
#endif
 
#ifndef __prm___message___buffer__
#define __prm___message___buffer__
 
struct prm___message___buffer {
  union {
    unsigned char  _prm___message___header[16]; /* Message Buffer Header */
    struct {
      int            _prm___msg___buffer___size;  /* Input - Size of buffer including the      */
      int            _prm___message___count;      /* Output - number of messages in the buffer */
      unsigned char  _prm___msg___buffer___flags;
      unsigned char  _filler1[7];                 /* Reserved                                  */
      } _prm___message___buffer_struct1;
    } _prm___message___buffer_union1;
  union {
    struct {
      unsigned char  _prm___messages; /* Messages */
      unsigned char  _filler2[255];
      } _prm___message___buffer_struct2;
    unsigned char  _prm___message___array[256];   /* Output - an array of messages        */
    unsigned char  _prm___message___element[256]; /* Output - information for one message */
    struct {
      unsigned char  _prm___msg___flags;         /* Output - indicator flags             */
      unsigned char  _filler3;                   /* Reserved                             */
      short int      _prm___msg___text___length; /* Output - length of this message text */
      unsigned char  _prm___msg___text[251];     /* Output - This message line's text    */
      unsigned char  _filler4;                   /* Reserved                             */
      } _prm___message___buffer_struct3;
    } _prm___message___buffer_union2;
  };
 
#define prm___message___header     _prm___message___buffer_union1._prm___message___header
#define prm___msg___buffer___size  _prm___message___buffer_union1._prm___message___buffer_struct1._prm___msg___buffer___size
#define prm___message___count      _prm___message___buffer_union1._prm___message___buffer_struct1._prm___message___count
#define prm___msg___buffer___flags _prm___message___buffer_union1._prm___message___buffer_struct1._prm___msg___buffer___flags
#define prm___messages             _prm___message___buffer_union2._prm___message___buffer_struct2._prm___messages
#define prm___message___array      _prm___message___buffer_union2._prm___message___array
#define prm___message___element    _prm___message___buffer_union2._prm___message___element
#define prm___msg___flags          _prm___message___buffer_union2._prm___message___buffer_struct3._prm___msg___flags
#define prm___msg___text___length  _prm___message___buffer_union2._prm___message___buffer_struct3._prm___msg___text___length
#define prm___msg___text           _prm___message___buffer_union2._prm___message___buffer_struct3._prm___msg___text
 
/* Values for field "prm___msg___buffer___flags" */
#define prm___msg___buffer___full    0x80  /* Output - Message buffer full */
 
/* Values for field "_filler4" */
#define prm___message___buffer___len 0x110
 
#endif
 
#pragma pack(reset)
