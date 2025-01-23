/*
  This program and the accompanying materials are
  made available under the terms of the Eclipse Public License v2.0 which accompanies
  this distribution, and is available at https://www.eclipse.org/legal/epl-v20.html

  SPDX-License-Identifier: EPL-2.0

  Copyright Contributors to the Zowe Project.
*/
#ifndef ZDSTYPE_H
#define ZDSTYPE_H

#include <stdint.h>
#include "ztype.h"

// RTNCD_CODE_SUCCESS ztype.h         -1
#define ZDS_RTNCD_SERVICE_FAILURE -2
#define ZDS_RTNCD_MAX_JOBS_REACHED -3
#define ZDS_RTNCD_INSUFFICIENT_BUFFER -4
#define ZDS_RTNCD_NOT_FOUND -5
#define ZDS_RTNCD_CATALOG_ERROR -6
#define ZDS_RTNCD_ENTRY_ERROR -7
#define ZDS_RTNCD_UNSUPPORTED_ERROR -8
#define ZDS_RTNCD_UNEXPECTED_ERROR -9
#define ZDS_RTNCD_PARSING_ERROR -10

#define ZDS_RSNCD_MAXED_ENTRIES_REACHED -1
#define ZDS_RSNCD_NOT_FOUND -2

#define ZDS_DEFAULT_BUFFER_SIZE 8096
#define ZDS_DEFAULT_MAX_ENTRIES 100

#define ZDS_VOLSER_VSAM "*VSAM*" // library
#define ZDS_VOLSER_ALIAS "*ALIAS"
#define ZDS_VOLSER_GDG "??????"
#define ZDS_VOLSER_UNKNOWN "------"

#define ZDS_DSORG_UNKNWON "--" // library
#define ZDS_DSORG_PDSE "PO-E"  // library
#define ZDS_DSORG_VSAM "VS"    // VSAM
#define ZDS_DSORG_PS "PS"      // sequential
#define ZDS_DSORG_PO "PO"      // partitioned

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(packed)
#endif

// NOTE(Kelosky): struct is padded to nearest double word boundary; ensure proper alignment for fields
typedef struct
{
  char eye[3];              // future use
  unsigned char version[1]; // future use
  int32_t len;              // future use

  char encoding[16];  // User-specified, desired encoding for dataset contents
  DataType data_type; // Desired data type for dataset contents (text or binary)

  int32_t max_entries;
  int32_t buffer_size;

  ZDIAG diag;

} ZDS;

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(reset)
#endif

#endif
