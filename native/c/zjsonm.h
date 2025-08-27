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

#ifndef ZJSONM_H
#define ZJSONM_H

#include <hwtjic.h> // ensure to include /usr/include
#include "ztype.h"
#include "zmetal.h"
#include "zwto.h"

typedef struct
{
  char x[12];
} PARSE_HANDLE;

typedef struct
{
  int x;
} KEY_HANDLE;

// typedef struct
// {
//   int y;
// } OBJECT_HANDLE;

typedef struct
{
  int z;
} ENTRY_VALUE_HANDLE;

typedef struct
{
  char msg[132];
} DIAG;

typedef int (*HWTJINIT)(int *PTR32, int *PTR32, PARSE_HANDLE *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*HWTJTERM)(int *PTR32, PARSE_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*HWTJPARS)(int *PTR32, PARSE_HANDLE *PTR32, char *PTR32 *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*HWTJSRCH)(int *PTR32, PARSE_HANDLE *PTR32, int *PTR32, char *PTR32 *PTR32, int *PTR32, int *PTR32, int *PTR32, KEY_HANDLE *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*HWTJGJST)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*HWTJGNUE)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*HWTJGVAL)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, char *PTR32 *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*HWTJGBOV)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, char *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*HWTJGAEN)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, int *PTR32, KEY_HANDLE *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*HWTJGOEN)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, int *PTR32, char *PTR32 *PTR32, int *PTR32, KEY_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjinit-initialize-instance
static int ZJSNMINIT(PARSE_HANDLE *PTR32 handle, DIAG *PTR32 diag)
{
  HWTJINIT hwtjinit = (HWTJINIT)load_module31("HWTJINIT");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  int max_size = 0;

  hwtjinit(&rc, &max_size, handle, diag_p);
  delete_module("HWTJINIT");
  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjpars-parse-json-string
// TODO(Kelosky): get return code EQUs
static int ZJSMPARS(PARSE_HANDLE *PTR32 handle, char *PTR32 json, DIAG *PTR32 diag)
{
  HWTJPARS hwtjpars = (HWTJPARS)load_module31("HWTJPARS");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  int json_file_size = (int)strlen(json);
  hwtjpars(&rc, handle, &json, &json_file_size, diag_p);
  delete_module("HWTJPARS");
  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjsrch-search
static int ZJSMSRCH(PARSE_HANDLE *PTR32 handle, char *PTR32 key, KEY_HANDLE *PTR32 key_handle, DIAG *PTR32 diag)
{
  HWTJSRCH hwtjsrch = (HWTJSRCH)load_module31("HWTJSRCH");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  int search_type = HWTJ_SEARCHTYPE_GLOBAL;
  int object_handle = 0;
  int starting_handle = 0;
  char *PTR32 key_p = key;
  int name_length = (int)strlen(key);
  int force = 0; // TODO(Kelosky): get return code EQUs

  hwtjsrch(&rc, handle, &search_type, &key_p, &name_length, &object_handle, &starting_handle, key_handle, diag_p);
  delete_module("HWTJSRCH");
  return rc;
}

static int ZJSNGJST(PARSE_HANDLE *PTR32 handle, KEY_HANDLE *PTR32 key_handle, int *PTR32 type, DIAG *PTR32 diag)
{
  HWTJGJST hwtjgjst = (HWTJGJST)load_module31("HWTJGJST");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgjst(&rc, handle, key_handle, type, diag_p);
  delete_module("HWTJGJST");
  return rc;
}

static int ZJSMGVAL(PARSE_HANDLE *PTR32 handle, KEY_HANDLE *PTR32 key_handle, char *PTR32 *PTR32 value, int *PTR32 value_length, DIAG *PTR32 diag)
{
  HWTJGVAL hwtjgval = (HWTJGVAL)load_module31("HWTJGVAL");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgval(&rc, handle, key_handle, value, value_length, diag_p);
  delete_module("HWTJGVAL");
  return rc;
}

static int ZJSMGNUE(PARSE_HANDLE *PTR32 handle, KEY_HANDLE *PTR32 key_handle, int *PTR32 number_entries, DIAG *PTR32 diag)
{
  HWTJGNUE hwtjgnue = (HWTJGNUE)load_module31("HWTJGNUE");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgnue(&rc, handle, key_handle, number_entries, diag_p);
  delete_module("HWTJGNUE");
  return rc;
}

static int ZJSMGBOV(PARSE_HANDLE *PTR32 handle, KEY_HANDLE *PTR32 key_handle, char *PTR32 value, DIAG *PTR32 diag)
{
  HWTJGBOV hwtjgbov = (HWTJGBOV)load_module31("HWTJGBOV");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgbov(&rc, handle, key_handle, value, diag_p);

  delete_module("HWTJGBOV");
  return rc;
}

static int ZJSMGAEN(PARSE_HANDLE *PTR32 handle, KEY_HANDLE *PTR32 key_handle, int *PTR32 index, KEY_HANDLE *PTR32 value, DIAG *PTR32 diag)
{
  HWTJGAEN hwtjgaen = (HWTJGAEN)load_module31("HWTJGAEN");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgaen(&rc, handle, key_handle, index, value, diag_p);
  delete_module("HWTJGAEN");
  return rc;
}

// https://www.ibm.com/docs/en/zos/3.1.0?topic=parser-hwtjterm-terminate-instance
static int ZJSNMTERM(PARSE_HANDLE *PTR32 handle, DIAG *PTR32 diag)
{
  HWTJTERM hwtjterm = (HWTJTERM)load_module31("HWTJTERM");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  int force = 0; // TODO(Kelosky): get return code EQUs

  hwtjterm(&rc, handle, &force, diag_p);
  delete_module("HWTJTERM");
  return rc;
}

static int ZJSMGOEN(PARSE_HANDLE *PTR32 handle, KEY_HANDLE *PTR32 key_handle, int *PTR32 index, char *PTR32 *PTR32 value, int *PTR32 value_length, KEY_HANDLE *PTR32 value_handle, int *PTR32 actual_length, DIAG *PTR32 diag)
{
  HWTJGOEN hwtjgoen = (HWTJGOEN)load_module31("HWTJGOEN");
  // TODO(Kelosky): add error handling

  int rc = 0;
  DIAG *PTR32 diag_p = diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgoen(&rc, handle, key_handle, index, value, value_length, value_handle, actual_length, diag_p);
  delete_module("HWTJGOEN");
  return rc;
}

#endif
