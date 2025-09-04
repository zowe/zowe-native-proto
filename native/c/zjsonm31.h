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
#include "zjsontype.h"

// NOTE(Kelosky): we can generate these from SYS1.MACLIB(HWTJKASM) in the future
#define HWT_Serv_JCREN 0x00000001
#define HWT_Serv_JGAEN 0x00000002
#define HWT_Serv_JGBOV 0x00000003
#define HWT_Serv_JGJST 0x00000004
#define HWT_Serv_JGNUE 0x00000005
#define HWT_Serv_JGOEN 0x00000006
#define HWT_Serv_JGVAL 0x00000007
#define HWT_Serv_JINIT 0x00000008
#define HWT_Serv_JPARS 0x00000009
#define HWT_Serv_JSERI 0x0000000A
#define HWT_Serv_JSRCH 0x0000000B
#define HWT_Serv_JTERM 0x0000000C
#define HWT_Serv_JGNUV 0x0000000D
#define HWT_Serv_JDEL 0x0000000E
#define HWT_Serv_JSENC 0x0000000F
#define HWT_Serv_JGENC 0x00000010
#define HWT_Serv_JOPTS 0x00000011

typedef int (*PTR32 HWTJINIT)(int *PTR32, int *PTR32, PARSE_HANDLE *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJGENC)(int *PTR32, PARSE_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJCREN)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, char *PTR32 *PTR32, int *PTR32, int *PTR32, char *PTR32 *PTR32, int *PTR32, KEY_HANDLE *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJDEL)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, KEY_HANDLE *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJSENC)(int *PTR32, PARSE_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJSERI)(int *PTR32, PARSE_HANDLE *PTR32, char *PTR32 *PTR32, int *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJTERM)(int *PTR32, PARSE_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJPARS)(int *PTR32, PARSE_HANDLE *PTR32, const char *PTR32 *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJSRCH)(int *PTR32, PARSE_HANDLE *PTR32, int *PTR32, const char *PTR32 *PTR32, int *PTR32, int *PTR32, int *PTR32, KEY_HANDLE *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJGJST)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJGNUE)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJGVAL)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, char *PTR32 *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJGBOV)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, char *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJGAEN)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, int *PTR32, KEY_HANDLE *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);
typedef int (*PTR32 HWTJGOEN)(int *PTR32, PARSE_HANDLE *PTR32, KEY_HANDLE *PTR32, int *PTR32, char *PTR32 *PTR32, int *PTR32, KEY_HANDLE *PTR32, int *PTR32, DIAG *PTR32) ATTRIBUTE(amode31);

#if defined(__IBM_METAL__)
#define GET_EP(index, ep)                                       \
  __asm(                                                        \
      "*                                                   \n"  \
      " L  15,16(0,0)    -> CVT                            \n"  \
      " L  15,544(15,0)  ->CVTCSRT                         \n"  \
      " L  15,84(15,0)   -> JSON WT                        \n"  \
      " L  15,4*%1(15,0) -> Requested service              \n"  \
      " ST  15,%0                                          \n"  \
      "*                                                      " \
      : "=m"(ep)                                                \
      : "i"(index)                                              \
      : "r15");
#else
#define GET_EP(index, ep)
#endif

static int zjsm_init(JSON_INSTANCE *PTR32 instance)
{
  HWTJINIT hwtjinit = NULL;
  GET_EP(HWT_Serv_JINIT, hwtjinit);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  int max_size = 0;

  hwtjinit(&rc, &max_size, &instance->handle, diag_p);
  return rc;
}

static int zjsm_get_encoding(JSON_INSTANCE *PTR32 instance, int *PTR32 encoding)
{
  HWTJGENC hwtjgenc = NULL;
  GET_EP(HWT_Serv_JGENC, hwtjgenc);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgenc(&rc, &instance->handle, encoding, diag_p);
  return rc;
}

static int zjsm_delete(JSON_INSTANCE *PTR32 instance, KEY_HANDLE *PTR32 key_handle, KEY_HANDLE *PTR32 value_handle)
{
  HWTJDEL hwtjdel = NULL;
  GET_EP(HWT_Serv_JDEL, hwtjdel);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);
  hwtjdel(&rc, &instance->handle, key_handle, value_handle, diag_p);
  return rc;
}

static int zjsm_set_encoding(JSON_INSTANCE *PTR32 instance, int *PTR32 encoding)
{
  HWTJSENC hwtjsenc = NULL;
  GET_EP(HWT_Serv_JSENC, hwtjsenc);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);
  hwtjsenc(&rc, &instance->handle, encoding, diag_p);
  return rc;
}

static int zjsm_serialize(JSON_INSTANCE *PTR32 instance, char *PTR32 buffer, int *PTR32 buffer_length, int *PTR32 buffer_length_actual)
{
  HWTJSERI hwtjseri = NULL;
  GET_EP(HWT_Serv_JSERI, hwtjseri);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjseri(&rc, &instance->handle, &buffer, buffer_length, buffer_length_actual, diag_p);
  return rc;
}

static int zjsm_parse(JSON_INSTANCE *PTR32 instance, const char *PTR32 json)
{
  HWTJPARS hwtjpars = NULL;
  GET_EP(HWT_Serv_JPARS, hwtjpars);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  int json_file_size = (int)strlen(json);
  hwtjpars(&rc, &instance->handle, &json, &json_file_size, diag_p);
  return rc;
}

static int zjsm_shallow_search(JSON_INSTANCE *PTR32 instance, const char *PTR32 key, KEY_HANDLE *PTR32 key_handle)
{
  HWTJSRCH hwtjsrch = NULL;
  GET_EP(HWT_Serv_JSRCH, hwtjsrch);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  int search_type = HWTJ_SEARCHTYPE_SHALLOW;
  int object_handle = 0;
  int starting_handle = 0;
  int name_length = (int)strlen(key);

  hwtjsrch(&rc, &instance->handle, &search_type, &key, &name_length, &object_handle, &starting_handle, key_handle, diag_p);
  return rc;
}

static int zjsm_get_type(JSON_INSTANCE *PTR32 instance, KEY_HANDLE *PTR32 key_handle, int *PTR32 type)
{
  HWTJGJST hwtjgjst = NULL;
  GET_EP(HWT_Serv_JGJST, hwtjgjst);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgjst(&rc, &instance->handle, key_handle, type, diag_p);
  return rc;
}

static int zjsm_get_string_value(JSON_INSTANCE *PTR32 instance, KEY_HANDLE *PTR32 key_handle, char *PTR32 *PTR32 value, int *PTR32 value_length)
{
  HWTJGVAL hwtjgval = NULL;
  GET_EP(HWT_Serv_JGVAL, hwtjgval);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgval(&rc, &instance->handle, key_handle, value, value_length, diag_p);
  return rc;
}

static int zjsm_get_number_of_entries(JSON_INSTANCE *PTR32 instance, KEY_HANDLE *PTR32 key_handle, int *PTR32 number_entries)
{
  HWTJGNUE hwtjgnue = NULL;
  GET_EP(HWT_Serv_JGNUE, hwtjgnue);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgnue(&rc, &instance->handle, key_handle, number_entries, diag_p);
  return rc;
}

static int zjsm_get_boolean_value(JSON_INSTANCE *PTR32 instance, KEY_HANDLE *PTR32 key_handle, char *PTR32 value)
{
  HWTJGBOV hwtjgbov = NULL;
  GET_EP(HWT_Serv_JGBOV, hwtjgbov);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgbov(&rc, &instance->handle, key_handle, value, diag_p);
  return rc;
}

static int zjsm_get_array_entry(JSON_INSTANCE *PTR32 instance, KEY_HANDLE *PTR32 key_handle, int *PTR32 index, KEY_HANDLE *PTR32 value)
{
  HWTJGAEN hwtjgaen = NULL;
  GET_EP(HWT_Serv_JGAEN, hwtjgaen);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgaen(&rc, &instance->handle, key_handle, index, value, diag_p);
  return rc;
}

static int zjsm_term(JSON_INSTANCE *PTR32 instance)
{
  HWTJTERM hwtjterm = NULL;
  GET_EP(HWT_Serv_JTERM, hwtjterm);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  int force = HWTJ_NOFORCE;

  hwtjterm(&rc, &instance->handle, &force, diag_p);
  return rc;
}

static int zjsm_get_object_entry(JSON_INSTANCE *PTR32 instance, KEY_HANDLE *PTR32 key_handle, int *PTR32 index, char *PTR32 *PTR32 value, int *PTR32 value_length, KEY_HANDLE *PTR32 value_handle, int *PTR32 actual_length)
{
  HWTJGOEN hwtjgoen = NULL;
  GET_EP(HWT_Serv_JGOEN, hwtjgoen);

  int rc = 0;
  DIAG *PTR32 diag_p = &instance->diag;
  diag_p = (DIAG * PTR32)((unsigned int)diag_p | 0x80000000);

  hwtjgoen(&rc, &instance->handle, key_handle, index, value, value_length, value_handle, actual_length, diag_p);
  return rc;
}

#endif
