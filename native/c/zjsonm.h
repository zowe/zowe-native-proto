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

#if defined(__cplusplus) && (defined(__IBMCPP__) || defined(__IBMC__))
extern "OS"
{
#elif defined(__cplusplus)
extern "C"
{
#endif

  int ZJSMINIT(JSON_INSTANCE *PTR64 instance);

  int ZJSMGENC(JSON_INSTANCE *PTR64 instance, int *PTR64 encoding);

  int ZJSMPARS(JSON_INSTANCE *PTR64 instance, const char *PTR64 json);

  int ZJSMSRCH(JSON_INSTANCE *PTR64 instance, const char *PTR64 key, KEY_HANDLE *PTR64 key_handle);

  int ZJSNGJST(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 type);

  int ZJSMGVAL(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, char *PTR64 *PTR64 value, int *PTR64 value_length);

  int ZJSMGNUE(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 number_entries);

  int ZJSMGBOV(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, char *PTR64 value);

  int ZJSMGAEN(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 index, KEY_HANDLE *PTR64 value);

  int ZJSMGOEN(JSON_INSTANCE *PTR64 instance, KEY_HANDLE *PTR64 key_handle, int *PTR64 index, char *PTR64 *PTR64 value, int *PTR64 value_length, KEY_HANDLE *PTR64 value_handle, int *PTR64 actual_length);

  int ZJSMTERM(JSON_INSTANCE *PTR64 instance);

#if defined(__cplusplus)
}
#endif

#endif
