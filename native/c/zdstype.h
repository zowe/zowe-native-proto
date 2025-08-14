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
#define ZDS_DEFAULT_MAX_ENTRIES 1000
#define ZDS_DEFAULT_MAX_MEMBER_ENTRIES 5000

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
#define ZDS_RECFM_FB "FB"   // Fixed Blocked
#define ZDS_RECFM_F "F"     // Fixed
#define ZDS_RECFM_VB "VB"   // Variable Blocked
#define ZDS_RECFM_V "V"     // Variable
#define ZDS_RECFM_U "U"     // Undefined
#define ZDS_RECFM_FBS "FBS" // Fixed Blocked Spanned
#define ZDS_RECFM_VBS "VBS" // Variable Blocked Spanned

#if (defined(__IBMCPP__) || defined(__IBMC__))
#if defined(SWIG)
#pragma pack(1)
#else
#pragma pack(packed)
#endif
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

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(reset)
#endif

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(1)
#endif

// https://www.ibm.com/docs/en/SSLTBW_2.2.0/pdf/dgt3s310.pdf, page 26
typedef struct DSCBFormat1
{
  char ds1fmtid;     // Format Identifier (0x2C)
  char ds1dssn[6];   // Data set serial number (0x2D)
  uint16_t ds1volsq; // Volume sequence number (0x33)
  char ds1credt[3];  // Creation date (0x35)
  char ds1expdt[3];  // Expiration date (0x38)
  uint8_t ds1noepv;  // Number of extents on volume (0x3B)
  uint8_t ds1nobdb;  // Number of bytes used in last directory block (0x3C)
  uint8_t ds1flag1;  // Flags byte (0x3D)
  char ds1syscd[13]; // System code (0x3E)
  char ds1refd[3];   // Date last referenced (0x4B)
  uint8_t ds1smsfg;  // System managed storage indicators (0x4E)
  char ds1scext[3];  // Secondary space extension (0x4F)
  uint16_t ds1dsorg; // Data set organization (0x52)
  uint8_t ds1recfm;  // Record format (0x54)
  char ds1optcd;     // Option Code (0x55)
  char ds1blkl[2];   // Block length (Type F unblocked records), or maximum block size (F blocked, U or V records) (0x56)
  char ds1lrecl[2];  // Logical record length (0x58)
  char ds1keyl;      // Key length (0 to 255) (0x5A)
  char ds1rkp[2];    // Relative key position (0x5B)
  char ds1dsind;     // Data set indicators (0x5D)
  char ds1scalo[4];  // Secondary allocation space parameters (0x5E)
  char ds1lstar[3];  // Last used track and block on track (TTR) (0x62)
  char ds1trbal[2];  // If not extended format, value from TRKCALC indicating space remaining on last track used (0x65)
  char _filler1;     // Reserved (0x66)
  char ds1ttthi;     // High order byte of track number on DS1LSTAR, valid if ds1large is on (0x68)
  char ds1exnts[30]; // Three extent fields (0x69)
  char ds1ptrds[5];  // Pointer (CCHHR) to a format-2 or format-3 DSCB, or zero - if this DSCB is a format-8 DSCB, its always the CCHHR of a format-9 DSCB (0x87)
  char ds1end;       // End of DSCB-1 (0x8C)
} DSCBFormat1;

typedef struct IndexableDSCBFormat1
{
  char ds1dsnam[44]; // Data set name (used as key)
  DSCBFormat1 dscb1; // Contents of DSCB-1
} IndexableDSCBFormat1;

#if (defined(__IBMCPP__) || defined(__IBMC__))
#pragma pack(reset)
#endif

#endif