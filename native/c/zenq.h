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

#ifndef ZENQ_H
#define ZENQ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__IBM_METAL__)
#define ENQ_MODEL(enqm)                                       \
  __asm(                                                      \
      "*                                                  \n" \
      " ENQ (,,,,),MF=L                                   \n" \
      "*                                                    " \
      : "DS"(enqm));
#else
#define ENQ_MODEL(enqm) void *enqm;
#endif

ENQ_MODEL(enq_model); // make this copy in static storage

#if defined(__IBM_METAL__)
#define DEQ_MODEL(deqm)                                       \
  __asm(                                                      \
      "*                                                  \n" \
      " DEQ (,,,,),MF=L                                   \n" \
      "*                                                    " \
      : "DS"(deqm));
#else
#define DEQ_MODEL(deqm) void *deqm;
#endif

DEQ_MODEL(deq_model); // make this copy in static storage

#if defined(__IBM_METAL__)
#define ENQ(qname, rname, rlen, rc, plist)                     \
  __asm(                                                       \
      "*                                                  \n"  \
      " ENQ (%1,"                                              \
      "%2,"                                                    \
      "E,"                                                     \
      "%3,"                                                    \
      "SYSTEMS),"                                              \
      "RET=USE,"                                               \
      "MF=(E,%4)                                           \n" \
      "*                                                  \n"  \
      " ST 15,%0               Save RC                    \n"  \
      "*                                                    "  \
      : "=m"(rc)                                               \
      : "m"(qname), "m"(rname), "m"(rlen), "m"(plist)          \
      : "r0", "r1", "r14", "r15");
#else
#define ENQ(qname, rname, rlen, rc, plist)
#endif

// TODO(Kelosky): handle UCB for RESERVE
#if defined(__IBM_METAL__)
#define DEQ(qname, rname, rlen, rc, plist)                     \
  __asm(                                                       \
      "*                                                  \n"  \
      " DEQ (%1,"                                              \
      "%2,"                                                    \
      "%3,"                                                    \
      "SYSTEMS),"                                              \
      "RET=HAVE,"                                              \
      "MF=(E,%4)                                           \n" \
      "*                                                   \n" \
      " ST 15,%0               Save RC                     \n" \
      "*                                                    "  \
      : "=m"(rc)                                               \
      : "m"(qname), "m"(rname), "m"(rlen), "m"(plist)          \
      : "r0", "r1", "r14", "r15");
#else
#define DEQ(qname, rname, rlen, rc)
#endif

#define MAX_QNAME_LENGTH 8
typedef struct
{
  unsigned char qname[MAX_QNAME_LENGTH];
} QNAME;

#define MAX_RNAME_LENGTH 255
typedef struct
{
  int rlen;
  unsigned char rname[MAX_RNAME_LENGTH];
} RNAME;

static int enq(QNAME *qname, RNAME *rname)
{
  int rc = 0;
  ENQ_MODEL(dsa_enq_model);  // stack var
  dsa_enq_model = enq_model; // copy model

  ENQ(qname->qname, rname->rname, rname->rlen, rc, dsa_enq_model);
  return rc;
}

static int deq(QNAME *qname, RNAME *rname)
{
  int rc = 0;
  DEQ_MODEL(dsa_deq_model);  // stack var
  dsa_deq_model = deq_model; // copy model

  DEQ(qname->qname, rname->rname, rname->rlen, rc, dsa_deq_model);
  return rc;
}

#endif