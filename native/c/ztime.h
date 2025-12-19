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

#ifndef ZTIME_H
#define ZTIME_H

#if defined(__IBM_METAL__)
#define TIME(tod)                                             \
  __asm(                                                      \
      "*                                                  \n" \
      " TIME STCK,%0                                      \n" \
      "*                                                    " \
      : "=m"(tod)                                             \
      :                                                       \
      : "r0", "r1", "r14", "r15");
#else
#define TIME(tod)
#endif

#if defined(__IBM_METAL__)
#define TIME_LOCAL(time, date)                                 \
  __asm(                                                       \
      "*                                                   \n" \
      " TIME DEC,ZONE=LT,LINKAGE=SVC                       \n" \
      "*                                                   \n" \
      " ST 0,%0                                            \n" \
      " ST 1,%1                                            \n" \
      "*                                                    "  \
      : "=m"(time), "=m"(date)                                 \
      :                                                        \
      : "r0", "r1", "r14", "r15");
#else
#define TIME_LOCAL(time, date)
#endif

#if defined(__IBM_METAL__)
#define STCKCONV_MODEL(stckconvm)      \
  __asm(                               \
      "*                           \n" \
      " STCKCONV MF=L              \n" \
      "*                           \n" \
      : "DS"(stckconvm));
#else
#define STCKCONV_MODEL(stckconvm)
#endif

STCKCONV_MODEL(stckconv_model);

#if defined(__IBM_METAL__)
#define STCKCOV(output, tod, plist, rc)                       \
  __asm(                                                      \
      "*                                                  \n" \
      " STCKCONV STCKVAL=%2,"                                 \
      "CONVVAL=%0,"                                           \
      "TIMETYPE=DEC,"                                         \
      "DATETYPE=MMDDYYYY,"                                    \
      "MF=(E,%3)                                          \n" \
      "*                                                  \n" \
      " ST    15,%1     Save RC                           \n" \
      "*                                                    " \
      : "=m"(output),                                         \
        "=m"(rc)                                              \
      : "m"(tod),                                             \
        "m"(plist)                                            \
      : "r0", "r1", "r14", "r15");
#else
#define STCKCOV(output, tod, plist, rc)
#endif

static void time(unsigned long long *tod)
{
  TIME(*tod);
}

typedef union
{
  unsigned int timei;
  struct
  {
    unsigned char HH;
    unsigned char MM;
    unsigned char SS;
    unsigned char unused;
  } times;
} TIME_UNION;

// TODO(Kelosky): this must be AMODE 31
static void time_local(unsigned int *time, unsigned int *date)
{
  TIME_LOCAL(*time, *date);
}

typedef struct
{
  unsigned char time[8];
  unsigned char month;   // MMDDYYYY
  unsigned char day;     // MMDDYYYY
  unsigned char year[2]; // MMDDYYYY
  unsigned char unused[4];
} TIME_STRUCT;

static int stckconv(unsigned long long *tod, TIME_STRUCT *time_struct)
{
  int rc = 0;

  STCKCONV_MODEL(dsa_stckconv_model);
  dsa_stckconv_model = stckconv_model;

  STCKCOV(*time_struct, *tod, dsa_stckconv_model, rc);

  return rc;
}

#endif
