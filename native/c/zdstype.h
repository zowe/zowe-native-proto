/**
 * This program and the accompanying materials are made available under the terms of the
 * Eclipse Public License v2.0 which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v20.html
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Copyright Contributors to the Zowe Project.
 *
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

#define ZDS_DSORG_UNKNOWN "--" // library
#define ZDS_DSORG_PDSE "PO-E"  // library
#define ZDS_DSORG_VSAM "VS"    // VSAM
#define ZDS_DSORG_PS "PS"      // sequential
#define ZDS_DSORG_PO "PO"      // partitioned

// Record format constants
#define ZDS_RECFM_FB "FB"     // Fixed Blocked
#define ZDS_RECFM_F "F"       // Fixed
#define ZDS_RECFM_VB "VB"     // Variable Blocked
#define ZDS_RECFM_V "V"       // Variable
#define ZDS_RECFM_U "U"       // Undefined
#define ZDS_RECFM_FBS "FBS"   // Fixed Blocked Spanned
#define ZDS_RECFM_VBS "VBS"   // Variable Blocked Spanned
#define ZDS_RECFM_UNKNOWN "?" // Unknown

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(packed)
#endif

// NOTE(Kelosky): struct is padded to nearest double word boundary; ensure proper alignment for fields
typedef struct
{
  char eye[3];              // future use
  unsigned char version[1]; // future use
  int32_t len;              // future use

  ZEncode encoding_opts;
  char etag[8];

  int32_t max_entries;
  int32_t buffer_size;

  void *PTR64 csi;

  ZDIAG diag;

} ZDS;

// Note(traeok): Not a full representation of DSCB Format-1/8, just the fields we need
// https://www.ibm.com/docs/en/SSLTBW_2.2.0/pdf/dgt3s310.pdf, page 26
typedef struct DSCBFormat1
{
  char ds1dsnam[44]; // Data set name
  char ds1fmtid;     // Format Identifier
  char ds1dssn[6];   // Data set serial number
  uint16_t ds1volsq; // Volume sequence number
  char ds1credt[3];  // Creation date
  char ds1expdt[3];  // Expiration date
  uint8_t ds1noepv;  // Number of extents on volume
  uint8_t ds1nobdb;  // Number of bytes used in last directory block
  uint8_t ds1flag1;  // Flags byte
  char ds1syscd[13]; // System code
  char ds1refd[3];   // Date last referenced
  uint8_t ds1smsfg;  // System managed storage indicators
  char ds1scext[3];  // Secondary space extension
  uint16_t ds1dsorg; // Data set organization
  char ds1recfm;     // Record format
  char ds1optcd;     // Option Code
} DSCBFormat1;

typedef struct ObtainCamlstSearchParams
{
  unsigned char function_code;
  unsigned char option_flags; // Contains bits for EADSCB, NOQUEUE, etc.
  unsigned short reserved;    // For alignment
  char *dsname_ptr;
  char *volume_ptr;
  void *workarea_ptr;
} ObtainCamlstSearchParams;

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(reset)
#endif

#endif
