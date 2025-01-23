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

#pragma pack(packed)

#ifndef __csifield__
#define __csifield__

struct csifield
{
  unsigned char csifiltk[44]; /* FILTER   KEY                */
  unsigned char csicatnm[44]; /* CATALOG NAME OR BLANKS      */
  unsigned char csiresnm[44]; /* RESUME NAME OR BLANKS       */
  struct
  {
    unsigned char _csidtyps[16]; /* ENTRY TYPES */
  } csidtypd;
  struct
  {
    unsigned char _csicldi;  /* RETURN D&I IF C A MATCH Y OR BLNK */
    unsigned char _csiresum; /* RESUME FLAG         Y OR BLANK    */
    unsigned char _csis1cat; /* SEARCH CATALOG      Y OR BLANK    */
    unsigned char _csioptns; /* OPTION FLAG FIELD @01A            */
  } csiopts;
  short int csinumen; /* NUMBER OF ENTRIES FOLLOWING */
  struct
  {
    unsigned char _csifldnm[8]; /* FIELD NAME */
  } csients;
};

#define csidtyps csidtypd._csidtyps
#define csicldi csiopts._csicldi
#define csiresum csiopts._csiresum
#define csis1cat csiopts._csis1cat
#define csioptns csiopts._csioptns
#define csifldnm csients._csifldnm

#endif

#pragma pack(reset)
