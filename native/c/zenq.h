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
#include "iefucbob.h"

typedef struct ucb UCB;

#if defined(__IBM_METAL__)
#define ENQ_MODEL(enqm)                                               \
  __asm(                                                              \
      "*                                                  \n"         \
      " ENQ (,,E,,SYSTEMS),MF=L                                   \n" \
      "*                                                    "         \
      : "DS"(enqm));
#else
#define ENQ_MODEL(enqm) void *enqm;
#endif

ENQ_MODEL(enq_model); // make this copy in static storage

#if defined(__IBM_METAL__)
#define RESERVE_MODEL(resm)                                   \
  __asm(                                                      \
      "*                                                  \n" \
      " RESERVE (,,E,,SYSTEMS),UCB=*-*,LOC=ANY,MF=L       \n" \
      "*                                                    " \
      : "DS"(resm));
#else
#define RESERVE_MODEL(resm) void *resm;
#endif

RESERVE_MODEL(reserve_model); // make this copy in static storage

// NOTE(Kelosky): this gives an S230 if not initialized properly which does not match the MF=E setting nor documentation
#if defined(__IBM_METAL__)
#define DEQ_MODEL(deqm)                                             \
  __asm(                                                            \
      "*                                                  \n"       \
      " DEQ (,,,SYSTEMS),MF=L                                   \n" \
      "*                                                    "       \
      : "DS"(deqm));
#else
#define DEQ_MODEL(deqm) void *deqm;
#endif

DEQ_MODEL(deq_model); // make this copy in static storage

#if defined(__IBM_METAL__)
#define ENQ(qname, rname, rlen, rc, plist)                     \
  __asm(                                                       \
      "*                                                  \n"  \
      " ENQ (%2,"                                              \
      "%3,"                                                    \
      "E,"                                                     \
      "(%4),"                                                  \
      "SYSTEMS),"                                              \
      "RET=USE,"                                               \
      "MF=(E,%0)                                           \n" \
      "*                                                  \n"  \
      " ST 15,%1               Save RC                    \n"  \
      "*                                                    "  \
      : "+m"(plist),                                           \
        "=m"(rc)                                               \
      : "m"(qname), "m"(rname), "r"(rlen)                      \
      : "r0", "r1", "r14", "r15");
#else
#define ENQ(qname, rname, rlen, rc, plist)
#endif

#if defined(__IBM_METAL__)
#define RESERVE(qname, rname, rlen, rc, ucb, plist)           \
  __asm(                                                      \
      "*                                                  \n" \
      " RESERVE (%2,"                                         \
      "%3,"                                                   \
      "E,"                                                    \
      "(%4),"                                                 \
      "SYSTEMS),"                                             \
      "RET=USE,"                                              \
      "UCB=%5,"                                               \
      "MF=(E,%0)                                          \n" \
      "*                                                  \n" \
      " ST 15,%1               Save RC                    \n" \
      "*                                                    " \
      : "+m"(plist),                                          \
        "=m"(rc)                                              \
      : "m"(qname), "m"(rname), "r"(rlen), "m"(ucb)           \
      : "r0", "r1", "r14", "r15");
#else
#define RESERVE(qname, rname, rlen, rc, ucb, plist)
#endif

#if defined(__IBM_METAL__)
#define DEQ(qname, rname, rlen, rc, plist)                     \
  __asm(                                                       \
      "*                                                  \n"  \
      " DEQ (%2,"                                              \
      "%3,"                                                    \
      "(%4),"                                                  \
      "SYSTEMS),"                                              \
      "RET=HAVE,"                                              \
      "MF=(E,%0)                                           \n" \
      "*                                                   \n" \
      " ST 15,%1               Save RC                     \n" \
      "*                                                    "  \
      : "+m"(plist),                                           \
        "=m"(rc)                                               \
      : "m"(qname), "m"(rname), "r"(rlen)                      \
      : "r0", "r1", "r14", "r15");
#else
#define DEQ(qname, rname, rlen, rc)
#endif

#if defined(__IBM_METAL__)
#define DEQ_RESERVE(qname, rname, rlen, rc, ucb, plist)        \
  __asm(                                                       \
      "*                                                  \n"  \
      " DEQ (%2,"                                              \
      "%3,"                                                    \
      "(%4),"                                                  \
      "SYSTEMS),"                                              \
      "RET=HAVE,"                                              \
      "UCB=%5,"                                                \
      "MF=(E,%0)                                           \n" \
      "*                                                   \n" \
      " ST 15,%1               Save RC                     \n" \
      "*                                                    "  \
      : "+m"(plist),                                           \
        "=m"(rc)                                               \
      : "m"(qname), "m"(rname), "r"(rlen), "m"(ucb)            \
      : "r0", "r1", "r14", "r15");
#else
#define DEQ_RESERVE(qname, rname, rlen, rc, ucb, plist)
#endif

typedef struct ucbcmext UCB;

#define MAX_QNAME_LENGTH 8
typedef struct
{
  unsigned char value[MAX_QNAME_LENGTH];
} QNAME;

#define MAX_RNAME_LENGTH 255
typedef struct
{
  int rlen;
  unsigned char value[MAX_RNAME_LENGTH];
} RNAME;

static int enq(QNAME *qname, RNAME *rname)
{
  int rc = 0;
  ENQ_MODEL(dsa_enq_model);  // stack var
  dsa_enq_model = enq_model; // copy model

  ENQ(qname->value, rname->value, rname->rlen, rc, dsa_enq_model);

  // NOTE(Kelosky): if RC != 0, RC is a pointer to the parameter list and byte 3 of the parameter list contains the return code :-(
  if (0 != rc)
  {
    unsigned char *byte_pointer = (unsigned char *PTR32)rc;
#define ENQ_RC_INDEX 3
    rc = byte_pointer[ENQ_RC_INDEX];
  }

  return rc;
}

static int reserve(QNAME *qname, RNAME *rname, UCB *ucb)
{
  int rc = 0;
  RESERVE_MODEL(dsa_reserve_model);  // stack var
  dsa_reserve_model = reserve_model; // copy model

  RESERVE(qname->value, rname->value, rname->rlen, rc, ucb, dsa_reserve_model);

  // NOTE(Kelosky): if RC != 0, RC is a pointer to the parameter list and byte 3 of the parameter list contains the return code :-(
  if (0 != rc)
  {
    unsigned char *byte_pointer = (unsigned char *PTR32)rc;
#define ENQ_RC_INDEX 3
    rc = byte_pointer[ENQ_RC_INDEX];
  }

  return rc;
}

static int deq(QNAME *qname, RNAME *rname)
{
  int rc = 0;
  DEQ_MODEL(dsa_deq_model);  // stack var
  dsa_deq_model = deq_model; // copy model

  DEQ(qname->value, rname->value, rname->rlen, rc, dsa_deq_model);

  // NOTE(Kelosky): if RC != 0, RC is a pointer to the parameter list and byte 3 of the parameter list contains the return code :-(
  if (0 != rc)
  {
    unsigned char *byte_pointer = (unsigned char *PTR32)rc;
#define DEQ_RC_INDEX 3
    rc = byte_pointer[DEQ_RC_INDEX];
  }

  return rc;
}

static int deq_reserve(QNAME *qname, RNAME *rname, UCB *ucb)
{
  int rc = 0;
  DEQ_RESERVE_MODEL(dsa_deq_reserve_model);  // stack var
  dsa_deq_reserve_model = deq_reserve_model; // copy model

  DEQ_RESERVE(qname->value, rname->value, rname->rlen, rc, ucb, dsa_deq_reserve_model);

  // NOTE(Kelosky): if RC != 0, RC is a pointer to the parameter list and byte 3 of the parameter list contains the return code :-(
  if (0 != rc)
  {
    unsigned char *byte_pointer = (unsigned char *PTR32)rc;
#define DEQ_RESERVE_RC_INDEX 3
    rc = byte_pointer[DEQ_RESERVE_RC_INDEX];
  }

  return rc;
}

#endif