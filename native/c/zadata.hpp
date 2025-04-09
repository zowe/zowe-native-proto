#ifndef ZADATA_HPP
#define ZADATA_HPP

#include <string>

typedef struct
{
  unsigned char language;
  unsigned short record_type;
  unsigned char arch_level;
  unsigned char flag;
  unsigned char edition;
  unsigned char reserved[4];
  unsigned short len;
} ADATA_HEADER;

typedef struct
{
  unsigned char esid[4];
  int statement_number;
  int location_counter;
  unsigned char reserved[8];
  int instruction_offset;
  int instruction_length;
  unsigned char instruction_value[3];
} MACHINE_RECORD;

typedef struct
{
  unsigned char esid[4];
  int statement_number;
  int input_record_number;
  int parent_record_number;
  int input_assigned_file_number;
  int parent_assigned_file_number;
  int location_counter;
  unsigned char input_record_origin;
  unsigned char parent_record_origin;
  unsigned char print_flags;
  unsigned char reserved0[2];
  unsigned char source_record_type;
  unsigned char assembler_operation_code;
  unsigned char flags;
  unsigned char reserved1[4];
  unsigned char address1[4];
  unsigned char reserved2[4];
  unsigned char address2[4];
  int offset_name_entry;
  int length_name_entry;
  int offset_operation;
  int length_operation;
  int offset_operand;
  int length_operand;
  int offset_remarks;
  int length_remarks;
  int offset_continuation_char;
  unsigned char reserved3[4];
  int input_macro_offset;
  int input_macro_length;
  int parent_macro_offset;
  int parent_macro_length;
  int source_record_offset;
  int source_record_length;
  unsigned char reserved4[8];
  char input_macro_name[1];
  char parent_macro_name[1];
  char source_record[1];
} SOURCE_ANALYSIS_RECORD;

#endif
